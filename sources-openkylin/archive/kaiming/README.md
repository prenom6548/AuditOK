# kaiming
Kaiming is the container application toolkit of openKylin.

## Dependencies

```shell
sudo apt install debhelper build-essential libglib2.0-dev libostree-dev libxml2-dev pkg-config cmake ninja-build libdbus-glib-1-dev nlohmann-json3-dev libdbus-1-dev libcurl4-gnutls-dev zlib1g-dev libelf-dev liblz4-dev liblzma-dev libdeflate-dev libzstd-dev uuid-dev libgtest-dev crun libyajl-dev
```
You can also check the "Build-Depends" provided in the debian/control file.

## Build

- **Debug build**

```shell
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```
- **Release build**
```shell
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

- **Debug build with coverage**
```shell
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -B build
cmake --build build
```

## License


Kaiming is licensed under the [GPL v2.0+](./LICENSE).

