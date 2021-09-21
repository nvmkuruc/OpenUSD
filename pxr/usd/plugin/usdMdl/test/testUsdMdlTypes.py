#!/pxrpythonsubst
#
# Copyright 2018 Pixar
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

import os
os.environ['PXR_USDMDL_PLUGIN_SEARCH_PATHS'] = os.path.join(os.getcwd(), 'mdl')

from pxr import Sdr, Gf
import unittest

class TestMDLTypes(unittest.TestCase):
    def _test_types(self, node, values):
        """
        Test instantiation of an MDL node and that it's default values are correct
        """
        n = Sdr.Registry().GetShaderNodeByIdentifierAndType(node, "mdl")
        self.assertIsNotNone(n)

        for name, def_value in values.items():
            self.assertEqual(n.GetInput(name).GetDefaultValue(), def_value)

    def test_MDLTypes(self):
        """
        Test MDL node-types not supported by default in Sdr.
        """

        self._test_types('::nvidia::test_types::test_OM_11505', { 
            'int2_parm' : Gf.Vec2i(2,1),
            'int3_parm' : Gf.Vec3i(3,0,1),
            'int4_parm' : Gf.Vec4i(4,0,0,1),
            'float2_parm' : Gf.Vec2f(2,1),
            'float3_parm' : Gf.Vec3f(3,0,1),
            'float4_parm' : Gf.Vec4f(4,0,0,1)
        })

if __name__ == '__main__':
    unittest.main()
