import Pyro4

dspDemo = False

dspURI = 'PYRO:pyroDSP@10.42.0.175:7000'

timeTest = [1, 2, 3]
pinsTest = [-1,-2,11]
valsTest = [9191, -4000, -3]

demoIntervalsOfTime = 100 # microseconds (1e-6 sec)

try:
    pyroDSP = Pyro4.Proxy(dspURI)

    if dspDemo:
        print('starting demo')
        pyroDSP.demo(demoIntervalsOfTime)
    else:
        print('creating action table file')
        pyroDSP.setProfile(timeTest, pinsTest, valsTest)
        pyroDSP.DownloadProfile()
        print('staring dsp')
        pyroDSP.trigCollect()

    print('end')
except Exception as e:
    print(e)
