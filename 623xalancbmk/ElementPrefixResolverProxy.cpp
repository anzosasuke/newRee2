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
// Class header file...
#include "xalanc/XPath/ElementPrefixResolverProxy.hpp"



#include <xalanc/XalanDOM/XalanElement.hpp>



#include <xalanc/PlatformSupport/DOMStringHelper.hpp>



#include <xalanc/DOMSupport/DOMServices.hpp>
#include <xalanc/DOMSupport/DOMSupport.hpp>



#include "xalanc/XPath/XPathEnvSupport.hpp"



XALAN_CPP_NAMESPACE_BEGIN



ElementPrefixResolverProxy::ElementPrefixResolverProxy(
			const XalanElement*		namespaceContext,
			const XPathEnvSupport&	envSupport,
			const DOMSupport& 		/* domSupport */,
            MemoryManagerType&      theManager) :
	m_namespaceContext(namespaceContext),
	m_envSupport(&envSupport),
	m_uri(theManager)
{
}



ElementPrefixResolverProxy::ElementPrefixResolverProxy(
			const XalanElement*		namespaceContext,
            MemoryManagerType&      theManager,
			const XPathEnvSupport*	envSupport) :
	m_namespaceContext(namespaceContext),
	m_envSupport(envSupport),
	m_uri(theManager)
{
}



ElementPrefixResolverProxy::~ElementPrefixResolverProxy()
{
}



const XalanDOMString*
ElementPrefixResolverProxy::getNamespaceForPrefix(const XalanDOMString&		prefix) const
{
	if (m_namespaceContext == 0)
	{
		return 0;
	}
	else
	{
		return DOMServices::getNamespaceForPrefix(prefix, *m_namespaceContext);
	}
}



const XalanDOMString&
ElementPrefixResolverProxy::getURI() const
{
	if (m_envSupport != 0 && m_namespaceContext != 0 && length(m_uri) == 0)
	{
#if defined(XALAN_NO_MUTABLE)
		((ElementPrefixResolverProxy*)this)->m_uri =
#else
		m_uri =
#endif
				m_envSupport->findURIFromDoc(m_namespaceContext->getOwnerDocument());
	}

	return m_uri;
}



XALAN_CPP_NAMESPACE_END
