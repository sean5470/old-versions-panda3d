""" DistributedInteractiveEntityAI module: contains the DistributedInteractiveEntityAI
    class, the server side representation of a simple, animated, interactive 
    prop."""


from AIBaseGlobal import *
from ClockDelta import *

import DirectNotifyGlobal
import FSM
import DistributedEntityAI
import State


class DistributedInteractiveEntityAI(DistributedEntityAI.DistributedEntityAI):
    """
    DistributedInteractiveEntityAI class:  The server side representation of
    an animated prop.  This is the object that remembers what the
    prop is doing.  The child of this object, the DistributedAnimatedProp
    object, is the client side version and updates the display that
    client's display based on the state of the prop.
    """

    if __debug__:
        notify = DirectNotifyGlobal.directNotify.newCategory('DistributedInteractiveEntityAI')

    def __init__(self, level, entId):
        """entId: a unique identifier for this prop."""
        DistributedEntityAI.DistributedEntityAI.__init__(self, level, entId)
        assert(self.debugPrint(
                "DistributedInteractiveEntityAI(entId=%s)"
                %(entId)))
        self.fsm = FSM.FSM('DistributedInteractiveEntityAI',
                           [State.State('off',
                                        self.enterOff,
                                        self.exitOff,
                                        ['playing']),
                            # Attract is an idle mode.  It is named attract
                            # because the prop is not interacting with an
                            # avatar, and is therefore trying to attract an
                            # avatar.
                            State.State('attract',
                                        self.enterAttract,
                                        self.exitAttract,
                                        ['playing']),
                            # Playing is for when an avatar is interacting
                            # with the prop.
                            State.State('playing',
                                        self.enterPlaying,
                                        self.exitPlaying,
                                        ['attract'])],
                           # Initial State
                           'off',
                           # Final State
                           'off',
                          )
        self.fsm.enterInitialState()
        self.avatarId=0


    def delete(self):
        del self.fsm
        DistributedEntityAI.DistributedEntityAI.delete(self)
    
    def getAvatarInteract(self):
        assert(self.debugPrint("getAvatarInteract() returning: %s"%(self.avatarId,)))
        return self.avatarId
    
    def getInitialState(self):
        assert(self.debugPrint("getInitialState()"))
        return [self.fsm.getCurrentState().getName(),
                globalClockDelta.getRealNetworkTime()]
    
    #def getOwnerDoId(self):
    #    assert(self.debugPrint("getOwnerDoId() returning: %s"%(self.ownerDoId,)))
    #    return self.ownerDoId
    
    def requestInteract(self):
        assert(self.debugPrint("requestInteract()"))
        avatarId = self.air.msgSender
        assert(self.notify.debug("  avatarId:%s"%(avatarId,)))
        stateName = self.fsm.getCurrentState().getName()
        if stateName != 'playing':
            self.sendUpdate("setAvatarInteract", [avatarId])
            self.avatarId=avatarId
            self.fsm.request('playing')
        else:
            self.sendUpdateToAvatarId(avatarId, "rejectInteract", [])
    
    def requestExit(self):
        assert(self.debugPrint("requestExit()"))
        avatarId = self.air.msgSender
        assert(self.notify.debug("  avatarId:%s"%(avatarId,)))
        if avatarId==self.avatarId:
            stateName = self.fsm.getCurrentState().getName()
            if stateName == 'playing':
                self.sendUpdate("avatarExit", [avatarId])
                self.fsm.request('attract')
        else:
            assert(self.notify.debug("  requestExit: invalid avatarId"))
    
    def getState(self):
        assert(self.debugPrint("getState()"))
        return [self.fsm.getCurrentState().getName(),
                globalClockDelta.getRealNetworkTime()]
    
    def d_setState(self, state):
        assert(self.debugPrint("d_setState(state=%s)"%(state,)))
        self.sendUpdate('setState', [state, globalClockDelta.getRealNetworkTime()])
    
    ##### off state #####
    
    def enterOff(self):
        assert(self.debugPrint("enterOff()"))
        #self.d_setState('off')
    
    def exitOff(self):
        assert(self.debugPrint("exitOff()"))
    
    ##### attract state #####
    
    def enterAttract(self):
        assert(self.debugPrint("enterAttract()"))
        self.d_setState('attract')
    
    def exitAttract(self):
        assert(self.debugPrint("exitAttract()"))
    
    ##### open state #####
    
    def enterPlaying(self):
        assert(self.debugPrint("enterPlaying()"))
        self.d_setState('playing')
    
    def exitPlaying(self):
        assert(self.debugPrint("exitPlaying()"))
    
    if __debug__:
        def debugPrint(self, message):
            """for debugging"""
            return self.notify.debug(
                    str(self.__dict__.get('entId', '?'))+' '+message)

