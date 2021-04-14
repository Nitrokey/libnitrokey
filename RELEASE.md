# Release guide

1. Bump version according to the [semver] rules, in format 
1. Start release branch: `release-<version>`
1. Update version in the following files:
    - meson.build
    - CMakeList.txt
    - libnitrokey.pro
3. Tag current HEAD as RC: `<version>-RC`
2. Confirm CI is passing, fix it otherwise
3. Run Python tests with handled devices (latest firmwares):
   - Nitrokey Pro (`test_pro.py`)
   - Nitrokey Storage (`test_storage.py`)
3. Merge release branch
3. Tag current HEAD as final: <version>
4. Provide signed source code archive as an artifact
5. Run a changelog generator, e.g. 
```bash
$ podman run -it --rm -v $PWD:/usr/local/src/your-app ferrarimarco/github-changelog-generator -u Nitrokey -p "libnitrokey" -t "<read_only_GH_token>" -o ""                             
```

[semver]: https://semver.org/