Quick & dirty Cython bindings for AM2302 / DHT22 temperature and humidity sensor on the Raspberry Pi 3b.

# Install

Install all Python 3 dependencies:

    pip3 install -r requirements.txt

Then run the `setup.py` script:

    python3 setup.py install

Run with the `--user` flag to install without root rights.

Tests can be executed using `pytest`. Note, that some reader tests might fail if data could not be read successfully from the sensor. This is currently expected. It should succeed at least every now and then. :)
