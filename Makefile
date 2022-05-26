NPROC=$(shell nproc)
IMAGE_NAME=libnk-build
BUILD_DIR=build

.PHONY: docker-build-all
docker-build-all:
	@echo "Running isolated build"
	$(MAKE) docker-build-image
	$(MAKE) docker-build

.PHONY: docker-build-image
docker-build-image:
	sudo docker build -t $(IMAGE_NAME) .

.PHONY: docker-build
docker-build:
	sudo docker run -it --rm -v $(PWD):/app $(IMAGE_NAME) bash -c "make ci-build"

DOCKERCMD=bash -c "make ci-package"
.PHONY: docker-package
docker-package:
	sudo docker run -it --rm -v $(PWD):/app $(IMAGE_NAME) $(DOCKERCMD)

.PHONY: docker-clean
docker-clean:
	cd $(BUILD_DIR) && $(MAKE) clean
	sudo docker rmi $(IMAGE_NAME) --force

.PHONY: ci-build
ci-build:
	mkdir -p $(BUILD_DIR) && rm -rf $(BUILD_DIR)/*
	cd $(BUILD_DIR) && cmake .. && $(MAKE) -j$(NPROC) package
	cd $(BUILD_DIR) && ctest -VV
	cd $(BUILD_DIR) && mkdir -p install && $(MAKE) install DESTDIR=install
	@echo "== Results available in $(BUILD_DIR)"
	@date

DGET_URL=https://people.debian.org/~patryk/tmp/libnitrokey/libnitrokey_3.7-1.dsc
.PHONY: ci-package
ci-package:
	mkdir -p $(BUILD_DIR) && rm -rf $(BUILD_DIR)/*
	cd $(BUILD_DIR) && dget $(DGET_URL)
	cd $(BUILD_DIR) && cd libnitrokey-* && dpkg-buildpackage

.PHONY: ci-tests
ci-tests:
	pip install pytest --user
	pip install -r unittest/requirements.txt --user
	cd unittest && python3 -m pytest -sv test_offline.py

REPORT_NAME=libnitrokey-tests-report.html
PYTEST_ARG=-vx --randomly-dont-reorganize --template=html1/index.html --report=$(REPORT_NAME)

tests-setup:
	cd unittest && pipenv --python $(shell which python3) &&  pipenv install --dev

.PHONY: tests-pro
tests-pro:
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_pro.py ; xdg-open $(REPORT_NAME)

.PHONY: tests-storage
tests-storage:
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_pro.py
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_storage.py ; xdg-open $(REPORT_NAME)

.PHONY: clean clean-all
clean:
	-rm $(REPORT_NAME)

clean-all: clean docker-clean