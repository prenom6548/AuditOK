[中文](README.md) | English

# openKylin Official Container Images

## Introduction

Dockerfiles for openKylin official container images, include openKylin basic image and application images.

## Base Images

The base images of openKylin are officially released by the community and are currently available at [openKylin repo](http://archive.build.openkylin.top/openkylin/). The `latest` tag represents the latest available long-term stable image.

Once the official images are released, they will be synchronized and pushed to various remote container image repositories, as detailed below:

### 1. Repository and Image Name
The {repository/name} for the base image on third-party image hubs is `openkylin/openkylin`

### 2. Supported Tags
* `2.0`

### 3. Contents
Base/openkylin/Dockerfile

## Application Images

Images for various popular application implementations based on openKylin base images.

### 1. Repository and Image Name
The {repository/name} for the application images on third-party image hubs is `openkylin/[application name]`

### 2. Contents
Application images are categorized into seven types based on scenarios, and the image building files are stored in corresponding scenario directories:
  - Bigdata:    `Bigdata/`
  - AI:       `AI/`
  - Distributed Storage: `Storage/`
  - Cloud Service:    `Cloud/`
  - Others:      `Others/`

#### 2.1 Overall Directory
Due to the complexity of application image functionalities, the depth of the directory is uncertain. The overall directory is as follows:
```
openkylin-docker-images/
└── AI/
    |── image-list.yml
	|── AppName_1/
	|    |── AppVersion_1/
	|	 |    └── OSVersion_1/
	|	 |    └── OSVersion_2/
	|	 └── AppVersion_1/
	|	      └── OSVersion_1/
	|	      └── OSVersion_2/
	|		  └── OSVersion_3/
	|───AppName_2/
	└───AppName_3/
```
Each scenario directory must contain an `image-list.yml` to describe the roots for each application image's **minimum directory unit (MDU)** (i.e., all `Image_i/`s in the example), `image-list.yml` is formatted as follows:
```
# image-list.yml示例
images:
	Image_1: Image_1/
	Image_2: Image_2/
	Image_3: Image_3/
```
The existence of `image-list.yml`:
- Helps check the integrity of the image directory.
```
# here, Image_i == nginx
nginx/
	|── README.md
	|── meta.yml
	|── doc/ (可选)
	|    |── picture/
	|	 |    └── logo.png
	|	 └── image-info.yml
	└── 1.28.1/
	     |── 2.0
		      └── Dockerfile
```
In each **MDU** `Image_i/`, the following contents must be included:
- Dockerfile

	The storage path is `[application version]/[openKylin version]/Dockerfile`. For example, the Dockerfile for the nginx application image version `1.28.0` based on openKylin `2.0` is located at `1.28.0/2.0/Dockerfile`.
- README.md

	The README should cover the following information in order:

    - Quick reference: Relevant link information
    - [Application Name] | openKylin: Description of the application's functionality
	- Supported tags and respective Dockerfile links: Description of the current application container image tags and Dockerfile links; must be updated when new images are added
    - Usage: Description of how to use the application container image, ideally including a simple runnable test case
    - Question and answering: Provide Issue links
 	
	The README will be published to the Overview or Description section of the third-party Hub image detail page; contributions via PR should be taken seriously.

- meta.yml

	This file describes the tag information and Dockerfile storage paths for each image, located at: `[application name]/meta.yml`. The file format is as follows:
    ```
	# nginx/meta.yml
	1.28.0-ok20: 
		path: nginx/1.28.0/2.0/Dockerfile
	```
    In the above example, each pair of `<key, value>` contains image's publishing configuration, it's as follows:
	- key: Represents the image tag, with a recommended naming format of: `[application version]-[openkylin version]`, such as `1.28.0-ok20`, indicating different tags of the `openkylin/nginx`.
	- value: Describes how to build the image, explained as follows:
	
		Configuration description:
		| Configuration Item | Required | Description | Example |
		|--|--|--|--|
		| path | yes | Relative path of the image dockerfile | nginx/1.28.1/2.0/Dockerfile |
		| architectures | yes |  Specifies the architecture when releasing a single-architecture image, optional `x86_64` or `aarch64` or `riscv64` | amd64,arm64,riscv64 |
	
	*Note: **When updating the image tag, the above configuration must also be updated**.

- (Optional) `doc/`

	This directory stores graphic and textual information about the image for display in openKylin Software Center's application image section. If it is not to be displayed in `openKylin Software Center`, this section can be omitted.

	- `doc/image-info.yml`

		The content is as follows:
			name
			category (options: bigdata, ai, storage, database, cloud, hpc, others)
			description
			tags
			download
			usage
			license

	- `doc/picture/`

		Store application-related images, such as application logos or runtime screenshots of typical scenes.
	
	*Note: **The files and contents in the above `doc/` directory must exist simultaneously for the image to be displayed correctly in `openKylin Software Center`**.

## Available Container Registries
Available container registries included:
2. [hub.docker.com](https://hub.docker.com/)
3. [quay.io](https://quay.io/)

For example, users can obtain the base image `openkylin/openkylin:latest` by the following commands
```
# pull image from hub.docker.com
docker pull openkylin/openkylin:latest

# pull from quay.io
docker pull quay.io/openkylin/openkylin:latest
```