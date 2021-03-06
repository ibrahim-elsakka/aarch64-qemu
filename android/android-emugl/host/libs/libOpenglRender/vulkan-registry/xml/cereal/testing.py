# Copyright (c) 2018 The Android Open Source Project
# Copyright (c) 2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from copy import deepcopy

from .common.codegen import CodeGen
from .common.vulkantypes import \
        VulkanAPI, makeVulkanTypeSimple, iterateVulkanType

from .wrapperdefs import VulkanWrapperGenerator
from .wrapperdefs import EQUALITY_VAR_NAMES
from .wrapperdefs import EQUALITY_ON_FAIL_VAR
from .wrapperdefs import EQUALITY_ON_FAIL_VAR_TYPE
from .wrapperdefs import EQUALITY_RET_TYPE
from .wrapperdefs import API_PREFIX_EQUALITY

class VulkanEqualityCodegen(object):

    def __init__(self, cgen, inputVars, onFailCompareVar, prefix):
        self.cgen = cgen
        self.inputVars = inputVars
        self.onFailCompareVar = onFailCompareVar
        self.prefix = prefix

        def makeAccess(varName, asPtr = True):
            return lambda t: self.cgen.generalAccess(t, parentVarName = varName, asPtr = asPtr)

        def makeLengthAccess(varName):
            return lambda t: self.cgen.generalLengthAccess(t, parentVarName = varName)

        self.exprAccessorLhs = makeAccess(self.inputVars[0])
        self.exprAccessorRhs = makeAccess(self.inputVars[1])

        self.exprAccessorValueLhs = makeAccess(self.inputVars[0], asPtr = False)
        self.exprAccessorValueRhs = makeAccess(self.inputVars[1], asPtr = False)

        self.lenAccessorLhs = makeLengthAccess(self.inputVars[0])
        self.lenAccessorRhs = makeLengthAccess(self.inputVars[1])

        self.checked = False

    def getTypeForCompare(self, vulkanType):
        res = deepcopy(vulkanType)

        if not vulkanType.accessibleAsPointer():
            res = res.getForAddressAccess()

        if vulkanType.staticArrExpr:
            res = res.getForAddressAccess()

        return res

    def makeCastExpr(self, vulkanType):
        return "(%s)" % (
            self.cgen.makeCTypeDecl(vulkanType, useParamName=False))

    def makeEqualExpr(self, lhs, rhs):
        return "(%s) == (%s)" % (lhs, rhs)

    def makeEqualBufExpr(self, lhs, rhs, size):
        return "(memcmp(%s, %s, %s) == 0)" % (lhs, rhs, size)

    def makeEqualStringExpr(self, lhs, rhs):
        return "(strcmp(%s, %s) == 0)" % (lhs, rhs)

    def makeBothNotNullExpr(self, lhs, rhs):
        return "(%s) && (%s)" % (lhs, rhs)

    def makeBothNullExpr(self, lhs, rhs):
        return "!(%s) && !(%s)" % (lhs, rhs)

    def compareWithConsequence(self, compareExpr, vulkanType, errMsg=""):
        self.cgen.stmt("if (!(%s)) { %s(\"%s (Error: %s)\"); }" %
                       (compareExpr, self.onFailCompareVar,
                        self.exprAccessorValueLhs(vulkanType), errMsg))

    def onCheck(self, vulkanType):

        self.checked = True

        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        bothNull = self.makeBothNullExpr(accessLhs, accessRhs)
        bothNotNull = self.makeBothNotNullExpr(accessLhs, accessRhs)
        nullMatchExpr = "(%s) || (%s)" % (bothNull, bothNotNull)

        self.compareWithConsequence( \
            nullMatchExpr,
            vulkanType,
            "Mismatch in optional field")

        skipStreamInternal = vulkanType.typeName == "void"

        if skipStreamInternal:
            return

        self.cgen.beginIf("%s && %s" % (accessLhs, accessRhs))

    def endCheck(self, vulkanType):

        skipStreamInternal = vulkanType.typeName == "void"
        if skipStreamInternal:
            return

        if self.checked:
            self.cgen.endIf()
            self.checked = False

    def onCompoundType(self, vulkanType):
        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        lenAccessLhs = self.lenAccessorLhs(vulkanType)
        lenAccessRhs = self.lenAccessorRhs(vulkanType)

        needNullCheck = vulkanType.pointerIndirectionLevels > 0

        if needNullCheck:
            bothNotNullExpr = self.makeBothNotNullExpr(accessLhs, accessRhs)
            self.cgen.beginIf(bothNotNullExpr)

        if lenAccessLhs is not None:
            equalLenExpr = self.makeEqualExpr(lenAccessLhs, lenAccessRhs)

            self.compareWithConsequence( \
                equalLenExpr,
                vulkanType, "Lengths not equal")

            loopVar = "i"
            accessLhs = "%s + %s" % (accessLhs, loopVar)
            accessRhs = "%s + %s" % (accessRhs, loopVar)
            forInit = "uint32_t %s = 0" % loopVar
            forCond = "%s < (uint32_t)%s" % (loopVar, lenAccessLhs)
            forIncr = "++%s" % loopVar

            if needNullCheck:
                self.cgen.beginIf(equalLenExpr)

            self.cgen.beginFor(forInit, forCond, forIncr)

        self.cgen.funcCall(None, self.prefix + vulkanType.typeName,
                           [accessLhs, accessRhs, self.onFailCompareVar])

        if lenAccessLhs is not None:
            self.cgen.endFor()
            if needNullCheck:
                self.cgen.endIf()

        if needNullCheck:
            self.cgen.endIf()

    def onString(self, vulkanType):
        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        bothNullExpr = self.makeBothNullExpr(accessLhs, accessRhs)
        bothNotNullExpr = self.makeBothNotNullExpr(accessLhs, accessRhs)
        nullMatchExpr = "(%s) || (%s)" % (bothNullExpr, bothNotNullExpr)

        self.compareWithConsequence( \
            nullMatchExpr,
            vulkanType,
            "Mismatch in string pointer nullness")

        self.cgen.beginIf(bothNotNullExpr)

        self.compareWithConsequence(
            self.makeEqualStringExpr(accessLhs, accessRhs),
            vulkanType, "Unequal strings")

        self.cgen.endIf()

    def onStringArray(self, vulkanType):
        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        lenAccessLhs = self.lenAccessorLhs(vulkanType)
        lenAccessRhs = self.lenAccessorRhs(vulkanType)

        bothNullExpr = self.makeBothNullExpr(accessLhs, accessRhs)
        bothNotNullExpr = self.makeBothNotNullExpr(accessLhs, accessRhs)
        nullMatchExpr = "(%s) || (%s)" % (bothNullExpr, bothNotNullExpr)

        self.compareWithConsequence( \
            nullMatchExpr,
            vulkanType,
            "Mismatch in string array pointer nullness")

        equalLenExpr = self.makeEqualExpr(lenAccessLhs, lenAccessRhs)

        self.compareWithConsequence( \
            equalLenExpr,
            vulkanType, "Lengths not equal in string array")

        self.compareWithConsequence( \
            equalLenExpr,
            vulkanType, "Lengths not equal in string array")

        self.cgen.beginIf("%s && %s" % (equalLenExpr, bothNotNullExpr))

        loopVar = "i"
        accessLhs = "*(%s + %s)" % (accessLhs, loopVar)
        accessRhs = "*(%s + %s)" % (accessRhs, loopVar)
        forInit = "uint32_t %s = 0" % loopVar
        forCond = "%s < (uint32_t)%s" % (loopVar, lenAccessLhs)
        forIncr = "++%s" % loopVar

        self.cgen.beginFor(forInit, forCond, forIncr)

        self.compareWithConsequence(
            self.makeEqualStringExpr(accessLhs, accessRhs),
            vulkanType, "Unequal string in string array")

        self.cgen.endFor()

        self.cgen.endIf()

    def onStaticArr(self, vulkanType):
        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        lenAccessLhs = self.lenAccessorLhs(vulkanType)

        finalLenExpr = "%s * %s" % (lenAccessLhs,
                                    self.cgen.sizeofExpr(vulkanType))

        self.compareWithConsequence(
            self.makeEqualBufExpr(accessLhs, accessRhs, finalLenExpr),
            vulkanType, "Unequal static array")

    def onPointer(self, vulkanType):
        skipStreamInternal = vulkanType.typeName == "void"
        if skipStreamInternal:
            return

        accessLhs = self.exprAccessorLhs(vulkanType)
        accessRhs = self.exprAccessorRhs(vulkanType)

        lenAccessLhs = self.lenAccessorLhs(vulkanType)
        lenAccessRhs = self.lenAccessorRhs(vulkanType)

        if lenAccessLhs is not None:
            self.compareWithConsequence( \
                self.makeEqualExpr(lenAccessLhs, lenAccessRhs),
                vulkanType, "Lengths not equal")

            finalLenExpr = "%s * %s" % (lenAccessLhs,
                                        self.cgen.sizeofExpr(
                                            vulkanType.getForValueAccess()))
        else:
            finalLenExpr = self.cgen.sizeofExpr(vulkanType.getForValueAccess())

        self.compareWithConsequence(
            self.makeEqualBufExpr(accessLhs, accessRhs, finalLenExpr),
            vulkanType, "Unequal dyn array")

    def onValue(self, vulkanType):
        accessLhs = self.exprAccessorValueLhs(vulkanType)
        accessRhs = self.exprAccessorValueRhs(vulkanType)
        self.compareWithConsequence(
            self.makeEqualExpr(accessLhs, accessRhs), vulkanType,
            "Value not equal")


class VulkanTesting(VulkanWrapperGenerator):

    def __init__(self, module, typeInfo):
        VulkanWrapperGenerator.__init__(self, module, typeInfo)

        self.codegen = CodeGen()

        self.equalityCodegen = \
            VulkanEqualityCodegen(
                None,
                EQUALITY_VAR_NAMES,
                EQUALITY_ON_FAIL_VAR,
                API_PREFIX_EQUALITY)

        self.knownDefs = {}

    def onGenType(self, typeXml, name, alias):
        VulkanWrapperGenerator.onGenType(self, typeXml, name, alias)

        if name in self.knownDefs:
            return

        category = self.typeInfo.categoryOf(name)

        if category in ["struct", "union"] and not alias:

            structInfo = self.typeInfo.structs[name]

            typeFromName = \
                lambda varname: makeVulkanTypeSimple(True, name, 1, varname)

            compareParams = \
                list(map(typeFromName, EQUALITY_VAR_NAMES)) + \
                [EQUALITY_ON_FAIL_VAR_TYPE]
                
            comparePrototype = \
                VulkanAPI(API_PREFIX_EQUALITY + name,
                          EQUALITY_RET_TYPE,
                          compareParams)

            def structCompareDef(cgen):
                self.equalityCodegen.cgen = cgen
                for member in structInfo.members:
                    iterateVulkanType(self.typeInfo, member,
                                      self.equalityCodegen)

            self.module.appendHeader(
                self.codegen.makeFuncDecl(comparePrototype))
            self.module.appendImpl(
                self.codegen.makeFuncImpl(comparePrototype, structCompareDef))

    def onGenCmd(self, cmdinfo, name, alias):
        VulkanWrapperGenerator.onGenCmd(self, cmdinfo, name, alias)

        # TODO: Figure something out for API testing
