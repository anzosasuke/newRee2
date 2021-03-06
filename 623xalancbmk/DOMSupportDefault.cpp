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
#include "xalanc/DOMSupport/DOMSupportDefault.hpp"



#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XalanDOM/XalanAttr.hpp>
#include <xalanc/XalanDOM/XalanDocument.hpp>
#include <xalanc/XalanDOM/XalanDocumentType.hpp>
#include <xalanc/XalanDOM/XalanElement.hpp>
#include <xalanc/XalanDOM/XalanEntity.hpp>
#include <xalanc/XalanDOM/XalanNamedNodeMap.hpp>



#include <xalanc/PlatformSupport/DOMStringHelper.hpp>
#include "xalanc/DOMSupport/DOMServices.hpp"
#include "xalanc/DOMSupport/DOMSupportException.hpp"



XALAN_CPP_NAMESPACE_BEGIN



DOMSupportDefault::DOMSupportDefault(MemoryManagerType&     theManager) :
	DOMSupport(),
	m_pool(theManager)
{
}



DOMSupportDefault::~DOMSupportDefault()
{
}



void
DOMSupportDefault::reset()
{
}



const XalanDOMString&
DOMSupportDefault::getUnparsedEntityURI(
			const XalanDOMString&	theName,
			const XalanDocument&	theDocument) const
{
	const XalanDocumentType* const	theDoctype =
		theDocument.getDoctype();

    XalanDOMStringPool&     theNonConstPool =
#if defined(XALAN_NO_MUTABLE)
	    ((DOMSupportDefault*)this)->m_pool;
#else
	    m_pool;
#endif

    MemoryManagerType&  theMemoryManager =
        theNonConstPool.getMemoryManager();

    XalanDOMString  theURI(theMemoryManager);

	if(theDoctype != 0)
	{
		const XalanNamedNodeMap* const	theEntities =
			theDoctype->getEntities();

		if (theEntities != 0)
		{
			const XalanNode* const	theNode =
				theEntities->getNamedItem(theName);

			if (theNode != 0 && theNode->getNodeType() == XalanNode::ENTITY_NODE)
			{
				const XalanEntity*	theEntity =
#if defined(XALAN_OLD_STYLE_CASTS)
					(const XalanEntity*)theNode;
#else
					static_cast<const XalanEntity*>(theNode);
#endif

                const XalanDOMString    theNotationName(
                                            theEntity->getNotationName(),
                                            theMemoryManager);

				if(isEmpty(theNotationName) == false) // then it's unparsed
				{
					// The draft says: "The XSLT processor may use the public
					// identifier to generate a URI for the entity instead of the URI
					// specified in the system identifier. If the XSLT processor does
					// not use the public identifier to generate the URI, it must use
					// the system identifier; if the system identifier is a relative
					// URI, it must be resolved into an absolute URI using the URI of
					// the resource containing the entity declaration as the base
					// URI [RFC2396]."
					// So I'm falling a bit short here.
					theURI = theEntity->getSystemId();

					if(isEmpty(theURI) == true)
					{
						theURI = theEntity->getPublicId();
					}
				}
			}
		}
	}

	return theNonConstPool.get(theURI);
}



bool
DOMSupportDefault::isNodeAfter(
			const XalanNode&	node1,
			const XalanNode&	node2) const
{
	return DOMServices::isNodeAfter(node1, node2);
}



XALAN_CPP_NAMESPACE_END
