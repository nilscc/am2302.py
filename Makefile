.PHONY: all build install test clean uninstall

all: install test

build:
	python3 setup.py build

install:
	python3 setup.py install --user

uninstall:
	pip3 uninstall am2302 -y

test:
	pytest

clean:
	rm -r build/ dist/ *.egg-info/ *.cpp *.c
