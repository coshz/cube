// swift-tools-version: 5.7
import PackageDescription

let package = Package(
    name: "Cube",
    platforms: [
        .iOS(.v13),
        .macOS(.v10_15)
    ],
    products: [
        .library(
            name: "Cube",
            targets: ["Cube"]
        )
    ],
    targets: [
        .binaryTarget(
            name: "Cube",
            url: "https://github.com/coshz/cube/releases/download/v0.5.3-alpha.2/cube.xcframework.zip",
            checksum: "e5b877dd1f4cb98bd7dc6a1edf7bb7d2c7dd68985fb70b99bbb8b643f6c8206b"
        )
    ]
)
