import am2302

def test_arrayAccess():
    am2302.setup()

    stream = am2302.Stream(pin=7)

    for i in range(3):

        # make sure all values are initialized to zero
        assert stream.timingsStart(i) == 0

        # assign new value
        stream.timingsStart(i, 5)
        assert stream.timingsStart(i) == 5

    for i in range(40):

        # make sure all values are initialized to zero
        assert stream.timingsHigh(i) == 0
        assert stream.timingsLow(i) == 0

        # assign new values
        stream.timingsHigh(i, 5)
        assert stream.timingsHigh(i) == 5
        stream.timingsLow(i, 4)
        assert stream.timingsLow(i) == 4

def test_valid():
    stream = am2302.Stream(pin=7)

    # valid measurement
    S = [22, 78, 82]
    H = [54, 54, 54, 54, 54, 54, 54, 54, 67, 54, 54, 54, 54, 53, 54, 54, 68, 54, 54, 54, 54, 54, 54, 54, 67, 54, 54, 54, 54, 54, 54, 54, 65, 54, 54, 54, 54, 54, 54, 54]
    L = [26, 26, 26, 26, 26, 26, 26, 73, 26, 26, 74, 26, 74, 74, 74, 72, 26, 26, 26, 26, 26, 26, 26, 73, 26, 26, 74, 26, 74, 26, 73, 25, 26, 73, 26, 74, 74, 26, 73, 73]

    for i,s in enumerate(S):
        stream.timingsStart(i,s)

    for i,(h,l) in enumerate(zip(H,L)):
        stream.timingsHigh(i,h)
        stream.timingsLow(i,l)

    stream.fillBits()
    stream.print()

    assert stream.valid()
    assert 29.8 == stream.temperature()
    assert 30.3 == stream.humidity()

def test_invalid_01():

    stream = am2302.Stream(pin=7)

    S = [22, 78, 82]
    #                                                        v-- defect
    H = [54, 53, 54, 54, 54, 54, 54, 54, 67, 54, 54, 54, 54, 46, 54, 54, 67, 54, 55, 54, 54, 54, 54, 54, 68, 54, 54, 55, 53, 54, 54,  54, 52, 54, 54, 54, 54, 54, 54, 47]
    L = [26, 26, 26, 26, 26, 26, 26, 73, 25, 26, 74, 26, 87, 68, 26, 73, 26, 26, 26, 26, 26, 26, 26, 72, 26, 26, 73, 26, 73, 74, 26, 116, 74, 26, 74, 73, 26, 74, 72, 1008]
    #                                            defect --^  ^-- defect                                                              ^-- defect

    for i,s in enumerate(S):
        stream.timingsStart(i,s)

    for i,(h,l) in enumerate(zip(H,L)):
        stream.timingsHigh(i,h)
        stream.timingsLow(i,l)

    stream.fillBits()
    stream.print()

    assert not stream.valid()
    assert 1 == stream.missingBits()

def test_invalid_02():

    stream = am2302.Stream(pin=7)

    S = [21, 78, 82]
    #                                        v-- defect
    H = [54, 54, 54, 54, 54, 54, 54, 53, 68, 134, 54, 54, 54, 53, 54, 67, 54, 54, 54, 54, 54, 54, 53, 68, 54, 54, 54, 55, 54, 54, 54, 65, 54, 54, 54, 54, 54, 54, 53, 47]
    L = [26, 26, 26, 26, 26, 26, 27, 73, 26,  25, 73, 74, 74, 73, 73, 26, 25, 26, 26, 26, 26, 27, 73, 26, 25, 73, 26, 73, 26, 74, 72, 26, 73, 26, 26, 74, 74, 26, 25, 1004]

    for i,s in enumerate(S):
        stream.timingsStart(i,s)

    for i,(h,l) in enumerate(zip(H,L)):
        stream.timingsHigh(i,h)
        stream.timingsLow(i,l)

    stream.fillBits()
    stream.print()

    assert not stream.valid()
    assert 1 == stream.missingBits()
