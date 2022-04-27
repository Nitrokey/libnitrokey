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
	sudo docker run -it -v $(PWD):/app $(IMAGE_NAME) bash -c "make ci-build"

.PHONY: docker-clean
docker-clean:
	sudo docker rmi $(IMAGE_NAME)

.PHONY: ci-build
ci-build:
	mkdir -p $(BUILD_DIR) && rm -rf $(BUILD_DIR)/*
	cd $(BUILD_DIR) && cmake .. && $(MAKE) -j$(NPROC)
	cd $(BUILD_DIR) && ctest -VV
	cd $(BUILD_DIR) && mkdir -p install && $(MAKE) install DESTDIR=install
	@echo "== Results available in $(BUILD_DIR)"
	@date

.PHONY: ci-tests
ci-tests:
	pip install pytest --user
	pip install -r unittest/requirements.txt --user
	cd unittest && python3 -m pytest -sv test_offline.py

PYTEST_ARG=-vx

.PHONY: tests-pro
tests-pro:
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_pro.py

.PHONY: tests-storage
tests-storage:
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_pro.py
	cd unittest && pipenv run pytest $(PYTEST_ARG) test_storage.py