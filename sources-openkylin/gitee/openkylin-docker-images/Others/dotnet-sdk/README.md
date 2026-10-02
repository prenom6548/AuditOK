# Quick reference

- The official Dotnet SDK (.NET SDK) docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Dotnet SDK (.NET SDK) | openKylin

Current Dotnet SDK (.NET SDK) docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

.NET is a free, cross-platform, open-source developer platform for building many kinds of applications. It can run programs written in multiple languages, with C# being the most popular. It relies on a high-performance sdk that is used in production by many high-scale apps.

Learn more about Dotnet SDK on [learn.microsoft.com](https://learn.microsoft.com/en-us/dotnet/core/introduction)⁠.

# Supported tags and respective Dockerfile links
The tag of each `dotnet-sdk` docker image is consist of the version of `dotnet-sdk` and the version of basic image. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[10.0.101-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/dotnet-sdk/10.0.101/2.0/Dockerfile) | dotnet 10.0.101 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

> Important: RISC-V support comes from the community, not official support from microsoft. Please be aware of the risks involved.

In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/dotnet-sdk` image from docker

	```bash
	docker pull openkylin/dotnet-sdk:{Tag}
	```

- Start a dotnet-sdk instance

	```bash
	docker run -d --name my-dotnet-sdk openkylin/dotnet-sdk:{Tag}
	```
- An example with HelloWorld application example

	1. Create a HelloWorld.csproj file with the following content
	
	       <Project Sdk="Microsoft.NET.Sdk">
			 <PropertyGroup>
				  <OutputType>Exe</OutputType>
				  <TargetFramework>net8.0</TargetFramework>
				  <ImplicitUsings>enable</ImplicitUsings>
				  <Nullable>enable</Nullable>
			 </PropertyGroup>
		   </Project>

	2. Create a Program.cs file with the following code
	
			Console.WriteLine("Hello, World!");

	3. Publish the .NET application (you need the "dotnet8" package)
	
			dotnet publish -c Release -o app
	
	4. Run the app with "openkylin/dotnet-sdk:10.0.101-ok203" 
	
			docker run --rm -v $PWD/app:/app openkylin/dotnet-sdk:10.0.101-ok20 /app/HelloWorld.dll 
			Hello, World!
			
- View container running logs

	```bash
	docker logs -f my-dotnet-sdk
	```

- To get an interactive shell

	```bash
	docker run -it --entrypoint /bin/bash --name my-dotnet-sdk openkylin/dotnet-sdk:{Tag}
	```
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).