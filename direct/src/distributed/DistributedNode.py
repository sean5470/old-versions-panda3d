"""DistributedNode module: contains the DistributedNode class"""

from ShowBaseGlobal import *
import NodePath
import DistributedObject
import Correction
import Prediction
import Task

class DistributedNode(DistributedObject.DistributedObject, NodePath.NodePath):
    """Distributed Node class:"""

    def __init__(self, cr):
        try:
            self.DistributedNode_initialized
        except:
            self.DistributedNode_initialized = 1
            self.DeadReconing = None
            DistributedObject.DistributedObject.__init__(self, cr)
        return None

    def disable(self):
        self.reparentTo(hidden)
        DistributedObject.DistributedObject.disable(self)

    def delete(self):
        self.reparentTo(hidden)
        DistributedObject.DistributedObject.delete(self)

    def setDeadReconing(self, state):
        self.DeadReconing = state
        if state:
            self.Predictor = NullPrediction(Point3(self.getX(), self.getY(),
                                                   self.getZ()))
            self.Corrector = SplineCorrection(Point3(self.getX(), self.getY(),
                                                     self.getZ()), Vec3(0))
            taskName = self.taskName("correctionPos")
            # remove any old tasks
            taskMgr.removeTasksNamed(taskName)
            # spawn new task
            task = Task.Task(self.correctPos)
            taskMgr.spawnTaskNamed(task, taskName)
        else:
            self.Predictor = None
            self.Corrector = None
            taskName = self.taskName("correctionPos")
            taskMgr.removeTasksNamed(taskName)

    def setPos(self, x, y, z):
        if self.DeadReconing:
            self.Predictor.newTelemetry(Point3(x, y, z))
        else:
            NodePath.NodePath.setPos(self, x, y, z)

    def setHpr(self, h, p, r):
        NodePath.NodePath.setHpr(self, h, p, r)

    def setPosHpr(self, x, y, z, h, p, r):
        if self.DeadReconing:
            self.Predictor.newTelemetry(Point3(x, y, z))
        else:
            NodePath.NodePath.setPosHpr(self, x, y, z, h, p, r)

    def d_setDeadReconing(self, state):
        self.sendUpdate("setDeadReconing", [state])

    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def d_broadcastPosHpr(self):
        
        self.d_setPosHpr(self.getX(), self.getY(), self.getZ(),
                         self.getH(), self.getP(), self.getR())

    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])

    def correctPos(self, task):
        self.Corrector.newTarget(self.Predictor.getPos(),
                                 self.Predictor.getVel())
        self.Corrector.step()
        NodePath.NodePath.setPos(self, self.Corrector.getPos())
        return Task.cont
