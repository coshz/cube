.PHONY: default prepare build dev 

default: build 

prepare:
	python scripts/sync_cxx.py 

build: prepare 
	python -m build 

dev: prepare 
	python -m pip install -e .

clean:
	rm -rf cxx dist
