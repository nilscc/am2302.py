import time
import am2302

def test_read():

    # sleep a lil so we don't overload the sensor
    time.sleep(1)

    assert not am2302.initialized()

    # read data
    d = am2302.read(retries=0)

    # everything should be initialized
    assert am2302.initialized()

    # result should be valid
    assert d is not None

    # some simple range checks
    assert d.humidity >= 0.0
    assert d.humidity <= 100.0
    assert d.temperature >= -50.0
    assert d.temperature <= 100.0


if __name__ == '__main__':
    test_read()
