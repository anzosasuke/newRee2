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
#include "xalanc/XSLT/ElemCopy.hpp"



#include <xercesc/sax/AttributeList.hpp>



#include <xalanc/PlatformSupport/XalanMessageLoader.hpp>



#include <xalanc/DOMSupport/DOMServices.hpp>



#include "xalanc/XSLT/Constants.hpp"
#include "xalanc/XSLT/Stylesheet.hpp"
#include "xalanc/XSLT/StylesheetConstructionContext.hpp"
#include "xalanc/XSLT/StylesheetExecutionContext.hpp"
#include "xalanc/XSLT/StylesheetRoot.hpp"
#include "xalanc/XSLT/TracerEvent.hpp"



XALAN_CPP_NAMESPACE_BEGIN



ElemCopy::ElemCopy(
            StylesheetConstructionContext&  constructionContext,
            Stylesheet&                     stylesheetTree,
            const AttributeListType&        atts,
            int                             lineNumber,
            int                             columnNumber) :
    ElemUse(constructionContext,
            stylesheetTree,
            lineNumber,
            columnNumber,
            StylesheetConstructionContext::ELEMNAME_COPY)
{
    const unsigned int  nAttrs = atts.getLength();

    for(unsigned int i = 0; i < nAttrs; i++)
    {
        const XalanDOMChar* const   aname = atts.getName(i);

        if(processUseAttributeSets(
                constructionContext,
                aname,
                atts,
                i) == false &&
           processSpaceAttr(
                Constants::ELEMNAME_COPY_WITH_PREFIX_STRING.c_str(),
                aname,
                atts,
                i,
                constructionContext) == false &&
           isAttrOK(
                aname,
                atts,
                i,
                constructionContext) == false)
        {
            error(
                constructionContext,
                XalanMessages::ElementHasIllegalAttribute_2Param,
                Constants::ELEMNAME_COPY_WITH_PREFIX_STRING.c_str(),
                aname);
        }
    }
}


const XalanDOMString&
ElemCopy::getElementName() const
{
    return Constants::ELEMNAME_COPY_WITH_PREFIX_STRING;
}


#if !defined(XALAN_RECURSIVE_STYLESHEET_EXECUTION)
const ElemTemplateElement*
ElemCopy::startElement(StylesheetExecutionContext&  executionContext) const
{
    XalanNode* const    sourceNode = executionContext.getCurrentNode();

    assert(sourceNode != 0);

    const XalanNode::NodeType   nodeType = sourceNode->getNodeType();

    if(XalanNode::DOCUMENT_NODE != nodeType)
    {
        executionContext.cloneToResultTree(
            *sourceNode,
            nodeType,
            true,
            false,
            getLocator());

        if(XalanNode::ELEMENT_NODE == nodeType)
        {
            ElemUse::startElement(executionContext);

            executionContext.copyNamespaceAttributes(*sourceNode);

            return beginExecuteChildren(executionContext);

        }
        else
        {
            if(0 != executionContext.getTraceListeners())
            {
                executionContext.fireTraceEvent(TracerEvent(executionContext,
                    *this));
            }
        }
    }
    else
    {
        if(0 != executionContext.getTraceListeners())
        {
            executionContext.fireTraceEvent(TracerEvent(executionContext,               
                *this));
        }

        ElemUse::startElement(executionContext);

        return beginExecuteChildren(executionContext);
    }  

    return 0;
}



void
ElemCopy::endElement(StylesheetExecutionContext& executionContext) const
{
    XalanNode* const    sourceNode = executionContext.getCurrentNode();

    assert(sourceNode != 0);

    const XalanNode::NodeType   nodeType = sourceNode->getNodeType();

    if(XalanNode::DOCUMENT_NODE != nodeType)
    {
        if(XalanNode::ELEMENT_NODE == nodeType)
        {
            endExecuteChildren(executionContext);

            executionContext.endElement(c_wstr(sourceNode->getNodeName()));

            ElemUse::endElement(executionContext);
        }
    }
    else
    {
        endExecuteChildren(executionContext);

        ElemUse::endElement(executionContext);
    }
}
#endif



#if defined(XALAN_RECURSIVE_STYLESHEET_EXECUTION)
void
ElemCopy::execute(StylesheetExecutionContext&   executionContext) const
{
    XalanNode* const    sourceNode = executionContext.getCurrentNode();

    assert(sourceNode != 0);

    const XalanNode::NodeType   nodeType = sourceNode->getNodeType();

    if(XalanNode::DOCUMENT_NODE != nodeType)
    {
        executionContext.cloneToResultTree(
            *sourceNode,
            nodeType,
            true,
            false,
            getLocator());

        if(XalanNode::ELEMENT_NODE == nodeType)
        {
            ElemUse::execute(executionContext);

            executionContext.copyNamespaceAttributes(*sourceNode);

            executeChildren(executionContext);

            executionContext.endElement(c_wstr(sourceNode->getNodeName()));
        }
        else
        {
            if(0 != executionContext.getTraceListeners())
            {
                executionContext.fireTraceEvent(TracerEvent(executionContext,
                    *this));
            }
        }
    }
    else
    {
        if(0 != executionContext.getTraceListeners())
        {
            executionContext.fireTraceEvent(TracerEvent(executionContext,               
                *this));
        }

        ElemUse::execute(executionContext);

        executeChildren(executionContext);
    }  
}
#endif



XALAN_CPP_NAMESPACE_END
