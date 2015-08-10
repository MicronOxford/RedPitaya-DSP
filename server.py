import Pyro4

def Abort():
    pass
def MoveAbsoluteADU():
    pass
def arcl():
    pass
def profileSet():
    pass
def DownloadProfile():
    pass
def InitProfile():
    pass
def trigCollect():
    pass
def ReadPosition():
    pass
def WriteDigital():
    pass

daemon = Pyro4.Daemon()
daemon.requestLoop()
