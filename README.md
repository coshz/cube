# Cube (Swift Package Manager Distribution)

This repository branch is dedicated to distributing the pre-compiled `Cube` XCFramework via Swift Package Manager (SPM).

> Our CI pipeline will update the `Package.swift` file on every release. Please do not modify it directly.

👉 For source code, documentation, and issue tracking, please visit the [main branch](https://github.com/coshz/cube).

## 📦 Installation

You can add `Cube` to your Xcode project using Swift Package Manager.

1. In Xcode, select **File** > **Add Package...** (or **Add Package Dependencies...** in newer versions)
2. Enter the repository URL in the search bar:
   ```text
   https://github.com/coshz/cube
   ```
3. Select 'Branch' in **Dependency Rule** dropdown, then type `spm-release` in the name field
4. Click **Add Package**.

Alternatively, if you are using a Package.swift file, add the following dependency:
```swift
dependencies: [
    .package(url: "https://github.com/coshz/cube", branch: "spm-release")
]
```
That's it!
