# Copyright (C) 2015 Tom Parks
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from __future__ import print_function

sec = 1e9
msec = 1e6
usec = 1e3
dt = 1e6#ns
f = open('acttable', 'w')

n = 100
t = 0
for l in range(n):
    print('{} {} {} {} {}'.format( t, 0, 0, 0, l*4000./n ), file=f)
    t += dt
    print('{} {} {} {} {}'.format( t, 0x000000FF, 0x000000FF, 4000, l*4000./n ), file=f)
    t += dt

#print('{} {} 13 0'.format( repr(int(t/1e9)), repr(int(t%1e9)) ), file=f, end='')
f.close()
!scp acttable root@192.168.1.100:/tmp/
!rm -f acttable

dt = 1e3#ns
f = open('acttable', 'w')

n = 1000000
t = 0
for l in range(n):
    print('{} {} {} {}'.format( t, 255 if l%2 else 0, 0, 4000*math.sin(l/100.) ), file=f)
    t += dt

#print('{} {} 13 0'.format( repr(int(t/1e9)), repr(int(t%1e9)) ), file=f, end='')
f.close()
!scp acttable root@192.168.1.100:/tmp/
!rm -f acttable


f = open('acttable', 'w')
print('{} {} 0'.format(0, 0xFFFFFFFF), file=f)
print('{} {} 1'.format(1e9, 0), file=f)
print('{} {} 0'.format(2e9, 0xFFFFFFFF), file=f)
f.close()

!scp acttable root@192.168.1.106:/tmp/
!rm -f acttable


# cam getint
dt = 1000
wait = 1e8-dt
n = 50
f = open('demotable', 'w')
t=0
for l in range(n):
    print('{} {} {} {} {}'.format( t, 0, 2, 0, 0 ), file=f)
    t += dt
    print('{} {} {} {} {}'.format( t, 0, 0, 0, 0 ), file=f)
    t += wait

!scp demotable root@192.168.1.100:/tmp/
!rm -f demotable

'''
    #print('{} 13 0'.format(repr(int(t))), file=f)
    #t += dt*3
    #print('{} 13 1'.format(repr(int(t))), file=f)
    #t += dt*3
'''


wait1 = 200
wait0 = 300
n = 2000000
f = open('demotable200300', 'w')
t = 0
for l in range(n):
    print('{} {} {}'.format( t, 9, 1 ), file=f)
    t += wait1
    print('{} {} {}'.format( t, 9, 0 ), file=f)
    t += wait0

# print('{} {} {}'.format( t, 9, 1 ), file=f)
# print('{} {} {}'.format( t, 11, -1 ), file=f)
# print('{} {} {}'.format( 2000000000, 9, 0 ), file=f)


wait = 500
n = 2000000
f = open('demotableWait500', 'w')
t = 500
for l in range(n):
    print('{} {} {}'.format( t, 9, 1 ), file=f)
    t += wait
    print('{} {} {}'.format( t, 9, 0 ), file=f)
    print('{} {} {}'.format( t, 11, -1 ), file=f)
    t = 500