#!/pxrpythonsubst
#
# Copyright 2017 Pixar
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.

# nv begin #fast-updates
import sys, unittest
from pxr import Sdf, Usd, Tf

class TestUsdFastUpdates(unittest.TestCase):
    def OnLayersChanged(self, notice, sender):
        self.layersChangedInvoked = True
        self.assertEqual(notice.GetLayers(), [self.layer])

    def OnObjectsChangedFastUpdates(self, notice, sender):
        self.OnObjectsChanged(notice, sender)
        # OM-8790
        #self.objectsChangedInvoked = True
        #fastUpdates = notice.GetFastUpdates()
        #self.assertEquals(len(fastUpdates), 1)
        #self.assertEquals(fastUpdates[0].path, Sdf.Path("/sphere.radius"))
        #self.assertEquals(fastUpdates[0].value, 6.0)
        #changedInfoPaths = notice.GetChangedInfoOnlyPaths()
        #self.assertEquals(len(changedInfoPaths ), 0)

    def OnObjectsChanged(self, notice, sender):
        self.objectsChangedInvoked = True
        fastUpdates = notice.GetFastUpdates()
        self.assertEquals(len(fastUpdates), 0)
        changedInfoPaths = notice.GetChangedInfoOnlyPaths()
        self.assertEquals(len(changedInfoPaths ), 1)
        self.assertEquals(changedInfoPaths[0], Sdf.Path("/sphere.radius"))

    def test_FastUpdatesNotice(self):
        # Set up basic stage.
        self.layersChangedInvoked = False
        self.objectsChangedInvoked = False
        self.layer = Sdf.Layer.CreateAnonymous()
        self.stage = Usd.Stage.Open(self.layer)
        assert self.stage
        sphere = self.stage.DefinePrim(Sdf.Path("/sphere"), "Sphere")
        assert sphere
        sphere.GetAttribute("radius").Set(9.0)

        # Verify expected notification with fast updates enabled.
        self._layersChangedListener = \
            Tf.Notice.RegisterGlobally(Sdf.Notice.LayersDidChange,
                                       self.OnLayersChanged)
        self._objectsChangedListener = \
            Tf.Notice.Register(Usd.Notice.ObjectsChanged,
                              self.OnObjectsChangedFastUpdates, self.stage)
        assert not self.layersChangedInvoked
        assert not self.objectsChangedInvoked
        with Sdf.ChangeBlock(True):
            sphere.GetAttribute("radius").Set(6.0)
        # OM-8790
        #assert self.layersChangedInvoked
        #assert self.objectsChangedInvoked

        # Verify expected notification with fast updates disabled.
        self._objectsChangedListener.Revoke()
        self.layersChangedInvoked = False
        self.objectsChangedInvoked = False
        self._objectsChangedListener = \
            Tf.Notice.Register(Usd.Notice.ObjectsChanged,
                              self.OnObjectsChanged, self.stage)
        with Sdf.ChangeBlock(False):
            sphere.GetAttribute("radius").Set(9.0)
        assert self.layersChangedInvoked
        assert self.objectsChangedInvoked

        # Verify no crash when removing a sublayer after authoring fast updates on it.
        self._layersChangedListener.Revoke()
        self._objectsChangedListener.Revoke()
        self.layer.Clear()
        self.stage = Usd.Stage.Open(self.layer)
        self.sublayer = Sdf.Layer.CreateAnonymous()
        self.layer.subLayerPaths.append(self.sublayer.identifier)
        with Usd.EditContext(self.stage, self.sublayer):
            sphere = self.stage.DefinePrim(Sdf.Path("/sphere"), "Sphere")
        with Usd.EditContext(self.stage, self.sublayer):
            with Sdf.ChangeBlock(True):
                sphere.GetAttribute("radius").Set(36.0)
        self.layer.subLayerPaths.remove(self.sublayer.identifier)

if __name__ == "__main__":
    unittest.main()
# nv end
