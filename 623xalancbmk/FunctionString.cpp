/*
 * Copyright 1999-2004 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "xalanc/XPath/FunctionString.hpp"



#include <xalanc/PlatformSupport/XalanMessageLoader.hpp>



#include <xalanc/DOMSupport/DOMServices.hpp>



#include "xalanc/XPath/XObjectFactory.hpp"



XALAN_CPP_NAMESPACE_BEGIN



FunctionString::FunctionString()
{
}



FunctionString::~FunctionString()
{
}



XObjectPtr
FunctionString::execute(
            XPathExecutionContext&  executionContext,
            XalanNode*              context,
            const LocatorType*      locator) const
{
    if (context == 0)
    {
        XPathExecutionContext::GetAndReleaseCachedString    theGuard(executionContext);
        XalanDOMString&     theResult = theGuard.get();

        executionContext.error(
            XalanMessageLoader::getMessage(
                theResult,
                XalanMessages::FunctionRequiresNonNullContextNode_1Param,
                "string"),
            context,
            locator);

        // Dummy return value...
        return XObjectPtr();
    }
    else
    {
        // The XPath standard says that if there are no arguments,
        // the argument defaults to a node set with the context node
        // as the only member.  The string value of a node set is the
        // string value of the first node in the node set.
        // DOMServices::getNodeData() will give us the data.

        // Get a cached string...
        XPathExecutionContext::GetAndReleaseCachedString    theData(executionContext);

        XalanDOMString&     theString = theData.get();

        DOMServices::getNodeData(*context, theString);

        return executionContext.getXObjectFactory().createString(theData);
    }
}



XObjectPtr
FunctionString::execute(
            XPathExecutionContext&  executionContext,
            XalanNode*              /* context */,          
            const XObjectPtr        arg1,
            const LocatorType*      /* locator */) const
{
    assert(arg1.null() == false);   

    if (arg1->getType() == XObject::eTypeString)
    {
        // Since XObjects are reference counted, just return the
        // argument.
        return arg1;
    }
    else
    {
        return executionContext.getXObjectFactory().createStringAdapter(arg1);
    }
}



#if defined(XALAN_NO_COVARIANT_RETURN_TYPE)
Function*
#else
FunctionString*
#endif
FunctionString::clone(MemoryManagerType&    theManager) const
{
    return XalanCopyConstruct(theManager, *this);
}



const XalanDOMString&
FunctionString::getError(XalanDOMString&    theResult) const
{
    return XalanMessageLoader::getMessage(
                theResult,
                XalanMessages::FunctionTakesZeroOrOneArg_1Param,
                "string()");
}



XALAN_CPP_NAMESPACE_END
