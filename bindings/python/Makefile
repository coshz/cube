.PHONY: default prepare build dev 

default: build 

prepare:
	python scripts/sync_cxx.py 

build: prepare 
	python -m build 

dev: prepare 
	python -m pip install -e .

test: dev
	python test/test.2.py

clean:
	python -m pip uninstall -y pycube
	rm -rf cxx dist
	find pycube \( -name "*.so" -o -name "*.pyd" \) -delete
