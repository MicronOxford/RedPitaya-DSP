# ActionTable runner for the Red Pitaya.

Run 'make all' to make the dsp, server, and download the os.  copy
everything from build/ to the SD card, and run server from a prompt on
the Red Pitaya.

Current version relies on Python 3 which we have compiled for the Red
Pitaya as the standard OS didn't have it at one point. To run a test service from the command line go to src/ and run

python3 server.py 

By default the system listens on 0.0.0.0:8005 so should be avalible on
all netowrk connections. When started it will display the Pyro URI.
It is easy to test with another network connected system. For example
with a red pitaya at 172.16.0.2:

```
 python3
 >>> import Pyro4
 >>> pp=Pyro4.Proxy("PYRO:redPitaya@172.16.0.2:8005")
 >>> pp.ReadPosition(0)
 124
 >>> pp.ReadDigital()
 7
 >>> pp.WriteDigital(3)
 >>> pp.ReadDigital()
 3
```


This tests that the server is running, and connections work and are
not blocked by firewalls or similar.


the Pyro URI will be displayed with a random port for testing.

If connecting with cockpit fails with a timeout error, this is
probably because the versions of Pyro on the cockpit machine and the
Red Pitaya do not match.


## License

See the [LICENSE](LICENSE) file for license rights and limitations (GPL version 3).