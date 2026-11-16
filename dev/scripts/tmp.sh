ROOT=$(git rev-parse --show-toplevel)
SWIFT_DIR="$ROOT/bindings/swift"

VERSION=$(cut -d'-' -f1 "$ROOT/VERSION")
echo $VERSION
