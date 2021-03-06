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
#include "xalanc/XalanTransformer/XalanTransformer.hpp"



#include <algorithm>
#if defined(XALAN_CLASSIC_IOSTREAMS)
#include <iostream.h>
#else
#include <iostream>
#endif



#include <xercesc/sax/SAXParseException.hpp>


#include <xalanc/Include/XalanMemMgrHelper.hpp>


#include <xalanc/XalanDOM/XalanDOMException.hpp>



#include <xalanc/PlatformSupport/DOMStringHelper.hpp>
#include <xalanc/PlatformSupport/DOMStringPrintWriter.hpp>
#include <xalanc/PlatformSupport/XalanMessageLoader.hpp>
#include <xalanc/PlatformSupport/XalanOutputStreamPrintWriter.hpp>



#include <xalanc/XPath/XObjectFactoryDefault.hpp>
#include <xalanc/XPath/XPathFactoryBlock.hpp>



#include <xalanc/XSLT/StylesheetConstructionContextDefault.hpp>
#include <xalanc/XSLT/StylesheetExecutionContextDefault.hpp>
#include <xalanc/XSLT/StylesheetRoot.hpp>
#include <xalanc/XSLT/XSLTEngineImpl.hpp>
#include <xalanc/XSLT/XSLTInit.hpp>
#include <xalanc/XSLT/XSLTProcessorEnvSupportDefault.hpp>



#include <xalanc/XalanSourceTree/XalanSourceTreeDOMSupport.hpp>
#include <xalanc/XalanSourceTree/XalanSourceTreeParserLiaison.hpp>



#include <xalanc/XalanExtensions/XalanExtensions.hpp>



#include <xalanc/XalanEXSLT/XalanEXSLTCommon.hpp>
#include <xalanc/XalanEXSLT/XalanEXSLTDynamic.hpp>
#include <xalanc/XalanEXSLT/XalanEXSLTMath.hpp>
#include <xalanc/XalanEXSLT/XalanEXSLTSet.hpp>
#include <xalanc/XalanEXSLT/XalanEXSLTString.hpp>
#include <xalanc/XalanEXSLT/XalanEXSLTDateTime.hpp>



//#define XALAN_USE_ICU
#if defined(XALAN_USE_ICU)
#include <xalanc/ICUBridge/ICUBridgeCleanup.hpp>
#include <xalanc/ICUBridge/ICUBridgeCollationCompareFunctor.hpp>
#include <xalanc/ICUBridge/ICUFormatNumberFunctor.hpp>
#endif



#include "xalanc/XalanTransformer/XalanCompiledStylesheetDefault.hpp"
#include "xalanc/XalanTransformer/XalanDefaultDocumentBuilder.hpp"
#include "xalanc/XalanTransformer/XalanDefaultParsedSource.hpp"
#include "xalanc/XalanTransformer/XalanTransformerOutputStream.hpp"
#include "xalanc/XalanTransformer/XalanTransformerProblemListener.hpp"
#include "xalanc/XalanTransformer/XercesDOMParsedSource.hpp"



XALAN_CPP_NAMESPACE_BEGIN


const XSLTInputSource*  XalanTransformer::s_emptyInputSource = 0;

const XSLTInit*         XalanTransformer::s_xsltInit = 0;


static MemoryManagerType* s_initMemoryManager = 0;

XalanTransformer::XalanTransformer(MemoryManagerType& theManager):
    m_memoryManager(theManager),
    m_compiledStylesheets(m_memoryManager),
    m_parsedSources(m_memoryManager),
    m_paramPairs(m_memoryManager),
    m_functionPairs(m_memoryManager),
    m_traceListeners(m_memoryManager),
    m_errorMessage(1, '\0', m_memoryManager),
    m_useValidation(false),
    m_entityResolver(0),
    m_errorHandler(0),
    m_externalSchemaLocation(m_memoryManager),
    m_externalNoNamespaceSchemaLocation(m_memoryManager),
    m_problemListener(0),
#if defined(XALAN_NO_STD_NAMESPACE)
    m_warningStream(&cerr),
#else
    m_warningStream(&std::cerr),
#endif
    m_outputEncoding(m_memoryManager),
    m_stylesheetExecutionContext(StylesheetExecutionContextDefault::create(m_memoryManager))
{
#if defined(XALAN_USE_ICU)
    // Create a collation function for the ICU, and have it
    // cache collators...
    XalanMemMgrAutoPtr<ICUBridgeCollationCompareFunctor, true>  theICUFunctor(m_memoryManager, ICUBridgeCollationCompareFunctor::create(m_memoryManager, true));

    m_stylesheetExecutionContext->installCollationCompareFunctor(theICUFunctor.get());

    XalanMemMgrAutoPtr<ICUFormatNumberFunctor, true>  theFormatNumberFunctor(m_memoryManager, ICUFormatNumberFunctor::create(m_memoryManager));
    m_stylesheetExecutionContext->installFormatNumberFunctor(theFormatNumberFunctor.get());
    theICUFunctor.release();
    theFormatNumberFunctor.release();

#endif
}



XalanTransformer::~XalanTransformer()
{
    XALAN_USING_STD(for_each)

    // Clean up all entries in the compliledStylesheets vector.
    for_each(m_compiledStylesheets.begin(),
             m_compiledStylesheets.end(),
             DeleteFunctor<XalanCompiledStylesheet>(m_memoryManager));

    // Clean up all entries in the compliledStylesheets vector.
    for_each(m_parsedSources.begin(),
             m_parsedSources.end(),
             DeleteFunctor<XalanParsedSource>(m_memoryManager));

    for (FunctionParamPairVectorType::size_type i = 0; i < m_functionPairs.size(); ++i)
    {
        if(m_functionPairs[i].second!= 0)
        {
            destroyObjWithMemMgr(
                m_functionPairs[i].second,
                m_memoryManager);
        }
    }

    typedef ParamPairVectorType::iterator iterator;

    for (iterator j = m_paramPairs.begin(); j != m_paramPairs.end(); ++j)
    {

        destroyObjWithMemMgr((*j).first, m_memoryManager);
        destroyObjWithMemMgr((*j).second, m_memoryManager);
    }

#if defined(XALAN_USE_ICU)
    // Uninstall the ICU collation compare functor, and destroy it...

    destroyObjWithMemMgr( m_stylesheetExecutionContext->uninstallCollationCompareFunctor(), m_memoryManager);
    destroyObjWithMemMgr( m_stylesheetExecutionContext->uninstallFormatNumberFunctor(), m_memoryManager);
#endif

    destroyObjWithMemMgr(
        m_stylesheetExecutionContext,
        m_memoryManager);
}



void
XalanTransformer::initialize(MemoryManagerType&  theManager)
{
    // Initialize Xalan. 
    XalanMemMgrAutoPtr<XSLTInit, true>          initGuard(theManager, XSLTInit::create(theManager));
    XalanAutoPtr<XSLTInputSource>   inputSourceGuard(new (&theManager) XSLTInputSource(theManager));
    EnsureFunctionsInstallation     instalGuard(theManager); 

    instalGuard.install();

    instalGuard.release();
    s_xsltInit = initGuard.releasePtr();
    s_emptyInputSource = inputSourceGuard.release();

    s_initMemoryManager = &theManager;
}



void
XalanTransformer::terminate()
{
    assert( s_initMemoryManager!= 0 );

    {
        EnsureFunctionsInstallation     uninstalGuard(*s_initMemoryManager);
    }

    delete s_emptyInputSource;

    destroyObjWithMemMgr(s_xsltInit, *s_initMemoryManager);

    s_emptyInputSource = 0;
    s_xsltInit = 0;
    s_initMemoryManager = 0;

#if defined(XALAN_USE_ICU)
    ICUBridgeCleanup::cleanup();
#endif
}



void
XalanTransformer::ICUCleanUp()
{
#if defined(XALAN_USE_ICU)
    ICUBridgeCleanup::cleanup();
#endif
}

#if defined(XALAN_USE_ICU)
void
ICUStartUp(MemoryManagerType&  theManager)
{
    ICUBridgeCleanup::startup(theManager);
}
#else
void
ICUStartUp(MemoryManagerType&  /*theManager*/)
{
}
#endif


static void
addTraceListeners(
             const XalanTransformer::TraceListenerVectorType&   theTraceListeners,
             XSLTEngineImpl&                                    theEngine)
{
    if (theTraceListeners.empty() == false)
    {
        typedef XalanTransformer::TraceListenerVectorType   TraceListenerVectorType;

        TraceListenerVectorType::const_iterator     theEnd = theTraceListeners.end();

        for(TraceListenerVectorType::const_iterator i = theTraceListeners.begin(); i != theEnd; ++i)
        {
            theEngine.addTraceListener(*i);
        }

        theEngine.setTraceSelects(true);
    }
}



int
XalanTransformer::transform(
            const XSLTInputSource&          theInputSource, 
            const XalanCompiledStylesheet*  theCompiledStylesheet,
            const XSLTResultTarget&         theResultTarget)
{
    const XalanParsedSource*    theParsedSource = 0;
 
    const int   theResult = parseSource(theInputSource, theParsedSource);
 
    if (theResult != 0)
    {
        return theResult;
    }
    else
    {
        assert(theParsedSource != 0);
 
        // Make sure the parsed source is destroyed when
        // the transformation is finished...
        EnsureDestroyParsedSource   theGuard(*this, theParsedSource);

        // Do the transformation...
        return transform(
                        *theParsedSource,
                        theCompiledStylesheet,
                        theResultTarget);
    }
}



int
XalanTransformer::transform(
            const XSLTInputSource&      theInputSource, 
            const XSLTInputSource&      theStylesheetSource,
            const XSLTResultTarget&     theResultTarget)
{
    // Parse the source document.
    const XalanParsedSource*    theParsedSource = 0;

    const int   theResult = parseSource(theInputSource, theParsedSource);

    if (theResult != 0)
    {
        return theResult;
    }
    else
    {
        assert(theParsedSource != 0);

        // Make sure the parsed source is destroyed when
        // the transformation is finished...
        EnsureDestroyParsedSource   theGuard(*this, theParsedSource);

        // Do the transformation...
        return transform(
                        *theParsedSource,
                        theStylesheetSource,
                        theResultTarget);
    }
}



int
XalanTransformer::transform(
            const XSLTInputSource&  theInputSource, 
            const XSLTInputSource&  theStylesheetSource,
            void*                   theOutputHandle, 
            XalanOutputHandlerType  theOutputHandler,
            XalanFlushHandlerType   theFlushHandler)
{
    // Set to output target to the callback 
    XalanTransformerOutputStream    theOutputStream(m_memoryManager, theOutputHandle, theOutputHandler, theFlushHandler);

    XalanOutputStreamPrintWriter    thePrintWriter(theOutputStream);

    XSLTResultTarget                theResultTarget(&thePrintWriter, m_memoryManager);

    // Do the transformation...
    return transform(
                    theInputSource, 
                    theStylesheetSource,
                    theResultTarget);
}



int
XalanTransformer::transform(
            const XalanParsedSource&        theParsedSource, 
            const XalanCompiledStylesheet*  theCompiledStylesheet,
            void*                           theOutputHandle, 
            XalanOutputHandlerType          theOutputHandler,
            XalanFlushHandlerType           theFlushHandler)
{
    // Set to output target to the callback 
    XalanTransformerOutputStream    theOutputStream(m_memoryManager, theOutputHandle, theOutputHandler, theFlushHandler);

    XalanOutputStreamPrintWriter    thePrintWriter(theOutputStream);

    XSLTResultTarget                theResultTarget(&thePrintWriter, m_memoryManager);

    // Do the transformation...
    return transform(
                    theParsedSource,
                    theCompiledStylesheet,
                    theResultTarget);
}



int
XalanTransformer::transform(
            const XSLTInputSource&      theInputSource,         
            void*                       theOutputHandle, 
            XalanOutputHandlerType      theOutputHandler,
            XalanFlushHandlerType       theFlushHandler)
{
    assert(s_emptyInputSource != 0);

    // Set to output target to the callback 
    XalanTransformerOutputStream    theOutputStream(m_memoryManager, theOutputHandle, theOutputHandler, theFlushHandler);

    XalanOutputStreamPrintWriter    thePrintWriter(theOutputStream);

    XSLTResultTarget                theResultTarget(&thePrintWriter, m_memoryManager);

    // Do the transformation...
    return transform(
                    theInputSource, 
                    *s_emptyInputSource,
                    theResultTarget);
}



XALAN_USING_XERCES(XMLException)
XALAN_USING_XERCES(SAXParseException)
XALAN_USING_XERCES(SAXException)



static const XalanDOMString&
FormatSAXParseException(
            XPathExecutionContext&      theExecutionContext,
            const SAXParseException&    theException,
            XalanDOMString&             theErrorMessage)
{
    assert(theException.getMessage() != 0);

    const XalanDOMChar  theDummy = 0;

    const XalanDOMChar* const   theSystemID = theException.getSystemId();

    typedef XPathExecutionContext::GetCachedString  GetCachedString;

    const GetCachedString   theGuard(theExecutionContext);

    XalanDOMString&     theExceptionType = theGuard.get();

    theExceptionType.assign("SAXParseException");

    XSLException::defaultFormat(
        theException.getMessage(),
        theSystemID == 0 ? &theDummy : theSystemID,
        theException.getLineNumber(),
        theException.getColumnNumber(),
        theExceptionType.c_str(),
        theErrorMessage);

    return theErrorMessage;
}



static void
FormatSAXParseException(
            XPathExecutionContext&      theExecutionContext,
            const SAXParseException&    theException,
            CharVectorType&             theErrorMessage)
{
    typedef XPathExecutionContext::GetCachedString  GetCachedString;

    const GetCachedString   theGuard(theExecutionContext);

    TranscodeToLocalCodePage(
        FormatSAXParseException(
            theExecutionContext,
            theException,
            theGuard.get()),
        theErrorMessage,
        true);
}



static void
FormatXalanDOMException(
            XPathExecutionContext&      theExecutionContext,
            const XalanDOMException&    theException,
            CharVectorType&             theErrorMessage)
{
    typedef XPathExecutionContext::GetCachedString  GetCachedString;

    const GetCachedString   theGuard1(theExecutionContext);
    const GetCachedString   theGuard2(theExecutionContext);

    const XalanDOMString&   theMessage =
        XalanMessageLoader::getMessage(
            theGuard1.get(),
            XalanMessages::XalanDOMExceptionCaught_1Param,
            LongToDOMString(
                theException.getExceptionCode(),
                theGuard2.get()));

    TranscodeToLocalCodePage(theMessage, theErrorMessage, true);
}



static void
LoadErrorMessage(
            XPathExecutionContext&      theExecutionContext,
            XalanMessages::Codes        theCode,
            CharVectorType&             theMessage)
{
    typedef XPathExecutionContext::GetCachedString  GetCachedString;

    const GetCachedString   theGuard(theExecutionContext);

    const XalanDOMString&   theMessageString =
                    XalanMessageLoader::getMessage(
                        theGuard.get(),
                        theCode);

    try
    {
        theMessageString.transcode(theMessage);
    }
    catch(...)
    {
        const char* const   theAlternateErrorMessage =
                "An error occurred transcoding an error message.";

        const XalanDOMString::size_type     theLength =
                length(theAlternateErrorMessage);

        theMessage.resize(theLength + 1);

#if defined(XALAN_STRICT_ANSI_HEADERS)
            XALAN_USING_STD(strncpy);
#endif

            strncpy(
                &*theMessage.begin(),
                theAlternateErrorMessage,
                theLength);
    }
}



int
XalanTransformer::compileStylesheet(
            const XSLTInputSource&              theStylesheetSource,
            const XalanCompiledStylesheet*&     theCompiledStylesheet)
{
    // Clear the error message.
    m_errorMessage.resize(1, '\0');

    // Store error messages from problem listener.
    XalanDOMString  theErrorMessage(m_memoryManager);

    int     theResult = 0;

    try
    {
        // Create some support objects that are necessary for running the processor...
        XalanSourceTreeDOMSupport       theDOMSupport;

        XalanSourceTreeParserLiaison    theParserLiaison(theDOMSupport, m_memoryManager);

        theParserLiaison.setEntityResolver(m_entityResolver);
        theParserLiaison.setErrorHandler(m_errorHandler);

        // Hook the two together...
        theDOMSupport.setParserLiaison(&theParserLiaison);

        // Create some more support objects...
        XSLTProcessorEnvSupportDefault  theXSLTProcessorEnvSupport(m_memoryManager);

        XObjectFactoryDefault   theXObjectFactory(m_memoryManager);

        XPathFactoryBlock       theXPathFactory(m_memoryManager);

        // Create a processor...
        XSLTEngineImpl  theProcessor(
                m_memoryManager,
                theParserLiaison,
                theXSLTProcessorEnvSupport,
                theDOMSupport,
                theXObjectFactory,
                theXPathFactory);

        // Create a problem listener and send output to a XalanDOMString.
        DOMStringPrintWriter    thePrintWriter(theErrorMessage);

        XalanTransformerProblemListener     theProblemListener( m_memoryManager, m_warningStream, &thePrintWriter);

        if (m_problemListener == 0)
        {
            theProcessor.setProblemListener(&theProblemListener);
        }
        else
        {
            theProcessor.setProblemListener(m_problemListener);
        }

        // Allocate the memory now, to avoid leaking if push_back() fails.
        m_compiledStylesheets.reserve(m_compiledStylesheets.size() + 1);

        // Create a new XalanCompiledStylesheet.
        theCompiledStylesheet =
            XalanCompiledStylesheetDefault::create(
                        m_memoryManager,
                        theStylesheetSource,
                        theProcessor,
                        m_errorHandler,
                        m_entityResolver);

        // Store it in a vector.
        m_compiledStylesheets.push_back(theCompiledStylesheet);
    }
    catch(const XSLException&   e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);
        }

        theResult = -1;
    }
    catch(const SAXParseException&  e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            FormatSAXParseException(
                *m_stylesheetExecutionContext,
                e,
                m_errorMessage);
        }

        theResult = -2;
    }
    catch(const SAXException&   e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);
        }

        theResult = -2;
    }
    catch(const XMLException&   e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);
        }

        theResult = -3;
    }
    catch(const XalanDOMException&  e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            FormatXalanDOMException(
                *m_stylesheetExecutionContext,
                e,
                m_errorMessage);
        }

        theResult = -4;
    }

    return theResult;
}



int
XalanTransformer::destroyStylesheet(const XalanCompiledStylesheet*  theStylesheet)
{
    XALAN_USING_STD(find)

    const CompiledStylesheetPtrVectorType::iterator     i =
        find(
            m_compiledStylesheets.begin(),
            m_compiledStylesheets.end(),
            theStylesheet);

    if (i == m_compiledStylesheets.end())
    {
        LoadErrorMessage(
            *m_stylesheetExecutionContext,
            XalanMessages::InvalidCompiledStylesheetProvided,
            m_errorMessage);

        return -1;
    }
    else
    {
        m_compiledStylesheets.erase(i);

        destroyObjWithMemMgr(
            theStylesheet,
            m_memoryManager);

        return 0;
    }
}



int
XalanTransformer::parseSource(
            const XSLTInputSource&      theInputSource,
            const XalanParsedSource*&   theParsedSource,
            bool                        useXercesDOM)
{
    // Clear the error message.
    m_errorMessage.clear();
    m_errorMessage.push_back(0);

    int theResult = 0;

    try
    {
        // Allocate the memory now, to avoid leaking if push_back() fails.
        m_parsedSources.reserve(m_parsedSources.size() + 1);

        if(useXercesDOM == true)
        {
            theParsedSource =
                XercesDOMParsedSource::create(
                        m_memoryManager,
                        theInputSource,
                        m_useValidation,
                        m_errorHandler,
                        m_entityResolver,
                        getExternalSchemaLocation(),
                        getExternalNoNamespaceSchemaLocation());
        }
        else
        {
            theParsedSource =
                XalanDefaultParsedSource::create(
                        m_memoryManager,
                        theInputSource,
                        m_useValidation,
                        m_errorHandler,
                        m_entityResolver,
                        getExternalSchemaLocation(),
                        getExternalNoNamespaceSchemaLocation());
        }

        // Store it in a vector.
        m_parsedSources.push_back(theParsedSource);
    }
    catch(const XSLException&   e)
    {
        XalanDOMString theBuffer(m_memoryManager);

        e.defaultFormat(theBuffer);

        TranscodeToLocalCodePage(theBuffer, m_errorMessage, true);

        theResult = -1;
    }
    catch(const SAXParseException&  e)
    {
        FormatSAXParseException(
            *m_stylesheetExecutionContext,
            e,
            m_errorMessage);

        theResult = -2;
    }
    catch(const SAXException&   e)
    {
        TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);

        theResult = -2;
    }
    catch(const XMLException&   e)
    {
        TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);

        theResult = -3;
    }
    catch(const XalanDOMException&  e)
    {
        FormatXalanDOMException(
            *m_stylesheetExecutionContext,
            e,
            m_errorMessage);

        theResult = -4;
    }

    return theResult;
}



int
XalanTransformer::destroyParsedSource(const XalanParsedSource*  theParsedSource)
{
    XALAN_USING_STD(find)

    const ParsedSourcePtrVectorType::iterator   i =
        find(
            m_parsedSources.begin(),
            m_parsedSources.end(),
            theParsedSource);

    if (i == m_parsedSources.end())
    {
        LoadErrorMessage(
            *m_stylesheetExecutionContext,
            XalanMessages::InvalidParsedSourceProvided,
            m_errorMessage);

        return -1;
    }
    else
    {
        m_parsedSources.erase(i);

        assert(theParsedSource != 0);

        destroyObjWithMemMgr(
            theParsedSource,
            m_memoryManager);

        return 0;
    }
}



void
XalanTransformer::setStylesheetParam(
            const XalanDOMString&   key,
            const XalanDOMString&   expression)
{
    m_paramPairs.push_back(
        ParamPairType(
            cloneObjWithMemMgr(
                key,
                m_memoryManager),
            cloneObjWithMemMgr(
                expression,
                m_memoryManager)));
}



void
XalanTransformer::setStylesheetParam(
            const char*     key,
            const char*     expression)
{

    setStylesheetParam(
        XalanDOMString(
            key,
            m_memoryManager),
        XalanDOMString(
            expression,
            m_memoryManager));
}



bool
XalanTransformer::removeTraceListener(TraceListener*    theTraceListener)
{
    XALAN_USING_STD(find)

    const TraceListenerVectorType::iterator     i =
        find(
            m_traceListeners.begin(),
            m_traceListeners.end(),
            theTraceListener);

    if (i == m_traceListeners.end())
    {
        return false;
    }
    else
    {
        m_traceListeners.erase(i);

        return true;
    }
}



XalanDocumentBuilder*
XalanTransformer::createDocumentBuilder(const XalanDOMString&   theURI)
{

    m_parsedSources.reserve(m_parsedSources.size() + 1);

    XalanDocumentBuilder* const     theNewBuilder = XalanDefaultDocumentBuilder::create( m_memoryManager, theURI);

    m_parsedSources.push_back(theNewBuilder);

    return theNewBuilder;
}



void
XalanTransformer::destroyDocumentBuilder(XalanDocumentBuilder*  theDocumentBuilder)
{
    destroyParsedSource(theDocumentBuilder);
}



void
XalanTransformer::installExternalFunction(
            const XalanDOMString&   theNamespace,
            const XalanDOMString&   functionName,
            const Function&         function)
{
    m_functionPairs.push_back(
        FunctionPairType(
            XalanQNameByValue::create(
                theNamespace,
                functionName,
                m_memoryManager),
            function.clone(m_memoryManager)));
}



void
XalanTransformer::installExternalFunctionGlobal(
            const XalanDOMString&   theNamespace,
            const XalanDOMString&   functionName,
            const Function&         function)
{
    XSLTProcessorEnvSupportDefault::installExternalFunctionGlobal(
            theNamespace,
            functionName,
            function);
}



void
XalanTransformer::uninstallExternalFunction(
            const XalanDOMString&   theNamespace,
            const XalanDOMString&   functionName)
{
    for (FunctionParamPairVectorType::size_type i = 0; i < m_functionPairs.size(); ++i)
    {
        if(m_functionPairs[i].first != 0 &&
            XalanQNameByReference(theNamespace, functionName).equals(* (m_functionPairs[i].first)))
        {
            destroyObjWithMemMgr( m_functionPairs[i].first, m_memoryManager);

            destroyObjWithMemMgr( m_functionPairs[i].second, m_memoryManager);

            m_functionPairs.erase(m_functionPairs.begin() + i);     
        }
    }   
}



void
XalanTransformer::uninstallExternalFunctionGlobal(
            const XalanDOMString&   theNamespace,
            const XalanDOMString&   functionName)
{
    XSLTProcessorEnvSupportDefault::uninstallExternalFunctionGlobal(
            theNamespace,
            functionName);
}



int
XalanTransformer::getIndent() const
{
    return m_stylesheetExecutionContext->getIndent();
}



void
XalanTransformer::setIndent(int indentAmount)
{
    m_stylesheetExecutionContext->setIndent(indentAmount);
}



XalanTransformer::eEscapeURLs
XalanTransformer::getEscapeURLs() const
{
    eEscapeURLs     escapeValue = eEscapeURLsDefault;

    switch(m_stylesheetExecutionContext->getEscapeURLs())
    {
    case StylesheetExecutionContextDefault::eEscapeURLsDefault:
        break;

    case StylesheetExecutionContextDefault::eEscapeURLsNo:
        escapeValue = eEscapeURLsNo;
        break;

    case StylesheetExecutionContextDefault::eEscapeURLsYes:
        escapeValue = eEscapeURLsYes;
        break;

    default:
        assert(false);
        break;
    }

    return escapeValue;
}



void
XalanTransformer::setEscapeURLs(eEscapeURLs     value)
{
    StylesheetExecutionContextDefault::eEscapeURLs  escapeValue =
        StylesheetExecutionContextDefault::eEscapeURLsDefault;

    switch(value)
    {
    case eEscapeURLsDefault:
        break;

    case eEscapeURLsNo:
        escapeValue = StylesheetExecutionContextDefault::eEscapeURLsNo;
        break;

    case eEscapeURLsYes:
        escapeValue = StylesheetExecutionContextDefault::eEscapeURLsYes;
        break;

    default:
        assert(false);
        break;
    }

    m_stylesheetExecutionContext->setEscapeURLs(escapeValue);
}



XalanTransformer::eOmitMETATag
XalanTransformer::getOmitMETATag() const
{
    eOmitMETATag    omitValue = eOmitMETATagDefault;

    switch(m_stylesheetExecutionContext->getOmitMETATag())
    {
    case StylesheetExecutionContextDefault::eOmitMETATagDefault:
        break;

    case StylesheetExecutionContextDefault::eOmitMETATagNo:
        omitValue = eOmitMETATagNo;
        break;

    case StylesheetExecutionContextDefault::eOmitMETATagYes:
        omitValue = eOmitMETATagYes;
        break;

    default:
        assert(false);
        break;
    }

    return omitValue;
}



void
XalanTransformer::setOmitMETATag(eOmitMETATag       value)
{
    StylesheetExecutionContextDefault::eOmitMETATag     omitValue =
        StylesheetExecutionContextDefault::eOmitMETATagDefault;

    switch(value)
    {
    case eOmitMETATagDefault:
        break;

    case eOmitMETATagNo:
        omitValue = StylesheetExecutionContextDefault::eOmitMETATagNo;
        break;

    case eOmitMETATagYes:
        omitValue = StylesheetExecutionContextDefault::eOmitMETATagYes;
        break;

    default:
        assert(false);
        break;
    }

    m_stylesheetExecutionContext->setOmitMETATag(omitValue);
}



void
XalanTransformer::reset()
{
    try
    {
        // Reset objects.
        m_stylesheetExecutionContext->setXPathEnvSupport(0);

        m_stylesheetExecutionContext->setDOMSupport(0);
        
        m_stylesheetExecutionContext->setXObjectFactory(0);
        
        m_stylesheetExecutionContext->setXSLTProcessor(0);

        m_stylesheetExecutionContext->reset();

        // Clear the ParamPairVectorType.
        XALAN_USING_STD(for_each)

/*        for_each(m_paramPairs.begin(),
                m_paramPairs.end(),
                DeleteParamPairFunctor<ParamPairType>(m_memoryManager));
*/
        typedef ParamPairVectorType::iterator iterator;

        for (iterator j = m_paramPairs.begin(); j != m_paramPairs.end(); ++j)
        {
    
            destroyObjWithMemMgr((*j).first, m_memoryManager);
            destroyObjWithMemMgr((*j).second, m_memoryManager);
        }     

        m_paramPairs.clear();
    }
    catch(...)
    {
    }
}



XalanTransformer::EnsureReset::~EnsureReset()
{
    m_transformer.m_stylesheetExecutionContext->reset();

    m_transformer.reset();
}



int
XalanTransformer::doTransform(
            const XalanParsedSource&        theParsedXML,
            const XalanCompiledStylesheet*  theCompiledStylesheet,
            const XSLTInputSource*          theStylesheetSource,
            const XSLTResultTarget&         theResultTarget)
{
    int     theResult = 0;

    // Clear the error message.
    m_errorMessage.resize(1, '\0');

    // Store error messages from problem listener.
    XalanDOMString  theErrorMessage(m_memoryManager);

    try
    {
        XalanDocument* const    theSourceDocument = theParsedXML.getDocument();
        assert(theSourceDocument != 0);

        // Create the helper object that is necessary for running the processor...
        XalanMemMgrAutoPtr<XalanParsedSourceHelper, true>   theHelper(m_memoryManager, theParsedXML.createHelper(m_memoryManager));

        assert(theHelper.get() != 0);

        DOMSupport&         theDOMSupport = theHelper->getDOMSupport();

        XMLParserLiaison&   theParserLiaison = theHelper->getParserLiaison();

        theParserLiaison.setExecutionContext(*m_stylesheetExecutionContext);

        theParserLiaison.setEntityResolver(m_entityResolver);
        theParserLiaison.setErrorHandler(m_errorHandler);
        theParserLiaison.setUseValidation(m_useValidation);

        // Create some more support objects...
        XSLTProcessorEnvSupportDefault  theXSLTProcessorEnvSupport(m_memoryManager);

        const XalanDOMString&   theSourceURI = theParsedXML.getURI();

        if (length(theSourceURI) > 0)
        {
            theXSLTProcessorEnvSupport.setSourceDocument(theSourceURI, theSourceDocument);
        }

        // Set the functions if any.
        {
            for (FunctionParamPairVectorType::size_type i = 0; i < m_functionPairs.size(); ++i)
            {
                assert( m_functionPairs[i].first != 0);
                assert( m_functionPairs[i].second != 0);

                theXSLTProcessorEnvSupport.installExternalFunctionLocal(
                        m_functionPairs[i].first->getNamespace(),
                        m_functionPairs[i].first->getLocalPart(),
                        *m_functionPairs[i].second);
            }
        }

        XObjectFactoryDefault   theXObjectFactory(m_memoryManager);

        XPathFactoryBlock       theXPathFactory(m_memoryManager);

        // Create a processor...
        XSLTEngineImpl  theProcessor(
                    m_memoryManager,
                    theParserLiaison,
                    theXSLTProcessorEnvSupport,
                    theDOMSupport,
                    theXObjectFactory,
                    theXPathFactory);

        theXSLTProcessorEnvSupport.setProcessor(&theProcessor);

        const EnsureReset   theReset(*this);

        // Set up the stylesheet execution context.
        m_stylesheetExecutionContext->setXPathEnvSupport(&theXSLTProcessorEnvSupport);

        m_stylesheetExecutionContext->setDOMSupport(&theDOMSupport);

        m_stylesheetExecutionContext->setXObjectFactory(&theXObjectFactory);

        m_stylesheetExecutionContext->setXSLTProcessor(&theProcessor);

        // Create a problem listener and send output to a XalanDOMString.  Do this before
        // pushing params, since there could be a problem resolving a QName.
        DOMStringPrintWriter    thePrintWriter(theErrorMessage);

        XalanTransformerProblemListener     theProblemListener(m_memoryManager, m_warningStream, &thePrintWriter);

        if (m_problemListener == 0)
        {
            theProcessor.setProblemListener(&theProblemListener);
        }
        else
        {
            theProcessor.setProblemListener(m_problemListener);
        }

        {
            // Set the parameters if any.
            for (ParamPairVectorType::size_type i = 0; i < m_paramPairs.size(); ++i)
            {
                assert ( m_paramPairs[i].first != 0);
                assert ( m_paramPairs[i].second != 0);

                theProcessor.setStylesheetParam(
                        * (m_paramPairs[i].first),
                        * (m_paramPairs[i].second));
            }
        }

        // Create an input source for the source document...
        XSLTInputSource     theDocumentInputSource(theSourceDocument, m_memoryManager);

        // Set the system ID, so relative URIs are resolved properly...
        theDocumentInputSource.setSystemId(c_wstr(theSourceURI));

        addTraceListeners(m_traceListeners, theProcessor);

        // We may need to change the output encoding, so
        // we make a copy of the result target.
        XSLTResultTarget    tempResultTarget(theResultTarget, m_memoryManager);

        if (tempResultTarget.getEncoding().length() == 0 && m_outputEncoding.length() != 0)
        {
            tempResultTarget.setEncoding(m_outputEncoding);
        }

        if (theCompiledStylesheet != 0)
        {
            assert(theStylesheetSource == 0 &&
                   theCompiledStylesheet->getStylesheetRoot() != 0);

            m_stylesheetExecutionContext->setStylesheetRoot(theCompiledStylesheet->getStylesheetRoot());

            // Do the transformation...
            theProcessor.process(
                        theDocumentInputSource,
                        tempResultTarget,
                        *m_stylesheetExecutionContext);
        }
        else
        {
            assert(theStylesheetSource != 0);

            // Create a stylesheet construction context, 
            // using the stylesheet's factory support objects.
            StylesheetConstructionContextDefault    theStylesheetConstructionContext(
                            m_memoryManager,
                            theProcessor,
                            theXPathFactory);

            // Do the transformation...
            theProcessor.process(
                            theDocumentInputSource,
                            *theStylesheetSource,
                            tempResultTarget,
                            theStylesheetConstructionContext,
                            *m_stylesheetExecutionContext);
        }
    }
    catch(const XSLException&   e)
    {
        if (length(theErrorMessage) == 0)
        {
            e.defaultFormat(theErrorMessage);
        }

        TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);

        theResult = -1;
    }
    catch(const SAXParseException&  e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            FormatSAXParseException(
                *m_stylesheetExecutionContext,
                e,
                m_errorMessage);
        }

        theResult = -2;
    }
    catch(const SAXException&   e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);
        }

        theResult = -2;
    }
    catch(const XMLException&   e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            TranscodeToLocalCodePage(e.getMessage(), m_errorMessage, true);
        }

        theResult = -3;
    }
    catch(const XalanDOMException&  e)
    {
        if (length(theErrorMessage) != 0)
        {
            TranscodeToLocalCodePage(theErrorMessage, m_errorMessage, true);
        }
        else
        {
            FormatXalanDOMException(
                *m_stylesheetExecutionContext,
                e,
                m_errorMessage);
        }

        theResult = -4;
    }

    return theResult;
}

void
XalanTransformer::EnsureFunctionsInstallation::install()
{
    XalanExtensionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTCommonFunctionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTDynamicFunctionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTMathFunctionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTSetFunctionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTStringFunctionsInstaller::installGlobal(m_memoryManagement);
    XalanEXSLTDateTimeFunctionsInstaller::installGlobal(m_memoryManagement);
}

XalanTransformer::EnsureFunctionsInstallation::~EnsureFunctionsInstallation()
{
    if ( !m_release)
    {
        XalanExtensionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTCommonFunctionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTDynamicFunctionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTMathFunctionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTSetFunctionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTStringFunctionsInstaller::uninstallGlobal(m_memoryManagement);
        XalanEXSLTDateTimeFunctionsInstaller::uninstallGlobal(m_memoryManagement);

#if defined(XALAN_USE_ICU)
        XPath::uninstallFunction(XPathFunctionTable::s_formatNumber);
#endif
    }
}


XALAN_CPP_NAMESPACE_END
