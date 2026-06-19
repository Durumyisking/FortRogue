# AGENTS.md

## Build Testing

When you need to build or verify Unreal code changes, use the project build batch file:

```bat
.\Build-UE57.bat
```

Use `--print` when you only need to verify the generated Unreal Build Tool command:

```bat
.\Build-UE57.bat --print
```

The batch file is configured for this project's UE 5.7 install:

```text
D:\Program Files\Epic Games\UE_5.7
```

Do not call another project's build script for FortRogue.
---

## Coding
- 언리얼 기능을 적극 활용하려고 노력하고, 언리얼 기능으로만 구현이 불가능할때 새로운 기능을 만든다.
- 하드코딩 방식이 아니라 가능한 에디터에서 추가 및 수정이 가능한 형태로 작성한다.