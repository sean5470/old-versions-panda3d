from pandac.PandaModules import *
from direct.distributed.DistributedObjectOV import DistributedObjectOV

class DistributedCameraOV(DistributedObjectOV):
    def __init__(self, cr):
        DistributedObjectOV.__init__(self, cr)
        self.parent = 0
        self.fixtures = []
        pass

    def delete(self):
        self.ignore('escape')
        DistributedObjectOV.delete(self)
        
    def getObject(self):
        return self.cr.getDo(self.getDoId())
        
    def setCamParent(self, doId):
        self.parent = doId
        pass
    
    def setFixtures(self, fixtures):
        self.fixtures = fixtures
        pass

    def storeToFile(self, name):
        f = file('cameras-%s.txt' % name, 'w')
        f.writelines(self.getObject().pack())
        f.close()
        pass
    
    def loadFromFile(self, name):
        self.b_setFixtures([])
        f = file('cameras-%s.txt' % name, 'r');
        for line in f.readlines():
            line = line.strip().replace('Camera(','').replace('))','')
            line = line.replace('Point3(','').replace('), VBase3(',', ')
            eval('self.addFixture([%s, \'Standby\'])' % line)
            pass
        f.close()
        pass

    def b_setFixtures(self, fixtures):
        self.getObject().setFixtures(fixtures)
        self.setFixtures(fixtures)
        self.d_setFixtures(fixtures)
        pass

    def d_setFixtures(self, fixtures):
        self.sendUpdate('setFixtures', [fixtures])
        pass

    def addFixture(self, fixture, index = None):
        if index is not None:
            self.fixtures.insert(index, fixture)
        else:
            self.fixtures.append(fixture)
            pass
        self.b_setFixtures(self.fixtures)
        return self.fixtures.index(fixture)

    def blinkFixture(self, index):
        if index < len(self.fixtures):
            fixture = self.fixtures[index]
            fixture[6] = 'Blinking'
            self.b_setFixtures(self.fixtures)
            pass
        pass

    def standbyFixture(self, index):
        if index < len(self.fixtures):
            fixture = self.fixtures[index]
            fixture[6] = 'Standby'
            self.b_setFixtures(self.fixtures)
            pass

    def testFixture(self, index):
        if index < len(self.fixtures):
            self.getObject().testFixture(index)
            pass

    def removeFixture(self, index):
        self.fixtures.pop(index)
        self.b_setFixtures(self.fixtures)
        pass

    def saveFixture(self, index = None):
        """
        Position the camera with ~oobe, then call this to save its telemetry.
        """
        parent = self.getObject().getCamParent()
        pos = base.cam.getPos(parent)
        hpr = base.cam.getHpr(parent)
        return self.addFixture([pos[0], pos[1], pos[2],
                                hpr[0], hpr[1], hpr[2],
                                'Standby'],
                               index)
        pass
    
    def startRecording(self):
        self.accept('escape', self.stopRecording)
        for fixture in self.fixtures:
            fixture[6] = 'Recording'
            pass
        self.b_setFixtures(self.fixtures)
        pass

    def stopRecording(self):
        self.ignore('escape')
        for fixture in self.fixtures:
            fixture[6] = 'Standby'
            pass
        self.b_setFixtures(self.fixtures)
        pass
