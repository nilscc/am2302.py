import time
import am2302

def test_Reader():

    # sleep a lil so we don't overload the sensor
    time.sleep(1)

    am2302.setup()
    assert am2302.initialized()

    reader = am2302.Reader()

    success = reader.run()

    # print some debug info
    print(reader.start1, reader.start2, reader.start3)
    print(reader.tempDone, reader.humidityDone, reader.parityDone)
    print(reader.temperature, reader.humidity, reader.parity)
    print(reader.awaitBit, reader.awaitLevel, reader.awaitDuration)

    # check if run was successful
    assert success

    # response should be valid as well
    assert reader.valid

if __name__ == '__main__':
    test_Reader()
