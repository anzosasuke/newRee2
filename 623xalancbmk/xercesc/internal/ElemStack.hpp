/*
 * Copyright 1999-2001,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: ElemStack.hpp 191054 2005-06-17 02:56:35Z jberry $
 */

#if !defined(ELEMSTACK_HPP)
#define ELEMSTACK_HPP

#include <xercesc/util/StringPool.hpp>
#include <xercesc/util/QName.hpp>
#include <xercesc/util/ValueVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLElementDecl;
class Grammar;

struct PrefMapElem : public XMemory
{
    unsigned int        fPrefId;
    unsigned int        fURIId;
};

//
//  During the scan of content, we have to keep up with the nesting of
//  elements (for validation and wellformedness purposes) and we have to
//  have places to remember namespace (prefix to URI) mappings.
//
//  We only have to keep a stack of the current path down through the tree
//  that we are currently scanning, and keep track of any children of any
//  elements along that path.
//
//  So, this data structure is a stack, which represents the current path
//  through the tree that we've worked our way down to. For each node in
//  the stack, there is an array of element ids that represent the ids of
//  the child elements scanned so far. Upon exit from that element, its
//  array of child elements is validated.
//
//  Since we have the actual XMLElementDecl in the stack nodes, when its time
//  to validate, we just extract the content model from that element decl
//  and validate. All the required data falls easily to hand. Note that we
//  actually have some derivative of XMLElementDecl, which is specific to
//  the validator used, but the abstract API is sufficient for the needs of
//  the scanner.
//
//  Since the namespace support also requires the storage of information on
//  a nested element basis, this structure also holds the namespace info. For
//  each level, the prefixes defined at that level (and the namespaces that
//  they map to) are stored.
//
class XMLPARSER_EXPORT ElemStack : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Class specific data types
    //
    //  These really should be private, but some of the compilers we have to
    //  support are too dumb to deal with that.
    //
    //  PrefMapElem
    //      fURIId is the id of the URI from the validator's URI map. The
    //      fPrefId is the id of the prefix from our own prefix pool. The
    //      namespace stack consists of these elements.
    //
    //  StackElem
    //      fThisElement is the basic element decl for the current element.
    //      The fRowCapacity is how large fChildIds has grown so far.
    //      fChildCount is how many of them are valid right now.
    //
    //      The fMapCapacity is how large fMap has grown so far. fMapCount
    //      is how many of them are valid right now.
    //
    //      Note that we store the reader number we were in when we found the
    //      start tag. We'll use this at the end tag to test for unbalanced
    //      markup in entities.
    //
    //  MapModes
    //      When a prefix is mapped to a namespace id, it matters whether the
    //      QName being mapped is an attribute or name. Attributes are not
    //      affected by an sibling xmlns attributes, whereas elements are
    //      affected by its own xmlns attributes.
    // -----------------------------------------------------------------------
    struct StackElem : public XMemory
    {
        XMLElementDecl*     fThisElement;
        unsigned int        fReaderNum;

        unsigned int        fChildCapacity;
        unsigned int        fChildCount;
        QName**             fChildren;

        PrefMapElem*        fMap;
        unsigned int        fMapCapacity;
        unsigned int        fMapCount;

        bool                fValidationFlag;
        bool                fCommentOrPISeen;
        bool                fReferenceEscaped;
#if defined(SPEC)
        unsigned int        fCurrentScope;
#else
        int                 fCurrentScope;
#endif
        Grammar*            fCurrentGrammar;
        unsigned int        fCurrentURI;
        XMLCh *             fSchemaElemName;
        unsigned int        fSchemaElemNameMaxLen;
        
        int                 fPrefixColonPos;
    };

    enum MapModes
    {
        Mode_Attribute
        , Mode_Element
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ElemStack(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ElemStack();


    // -----------------------------------------------------------------------
    //  Stack access
    // -----------------------------------------------------------------------
    unsigned int addLevel();
    unsigned int addLevel(XMLElementDecl* const toSet, const unsigned int readerNum);
    const StackElem* popTop();


    // -----------------------------------------------------------------------
    //  Stack top access
    // -----------------------------------------------------------------------
    unsigned int addChild(QName* const child, const bool toParent);
    const StackElem* topElement() const;
    void setElement(XMLElementDecl* const toSet, const unsigned int readerNum);

    void setValidationFlag(bool validationFlag);
    bool getValidationFlag();

    inline void setCommentOrPISeen();
    inline bool getCommentOrPISeen() const;

    inline void setReferenceEscaped();
    inline bool getReferenceEscaped() const;

    void setCurrentScope(int currentScope);
    int getCurrentScope();

    void setCurrentGrammar(Grammar* currentGrammar);
    Grammar* getCurrentGrammar();

    void setCurrentURI(unsigned int uri);
    unsigned int getCurrentURI();

    inline void setCurrentSchemaElemName(const XMLCh * const schemaElemName);
    inline XMLCh *getCurrentSchemaElemName();

    void setPrefixColonPos(int colonPos);
    int getPrefixColonPos() const;

    // -----------------------------------------------------------------------
    //  Prefix map methods
    // -----------------------------------------------------------------------
    void addPrefix
    (
        const   XMLCh* const    prefixToAdd
        , const unsigned int    uriId
    );
    unsigned int mapPrefixToURI
    (
        const   XMLCh* const    prefixToMap
        , const MapModes        mode
        ,       bool&           unknown
    )   const;
    ValueVectorOf<PrefMapElem*>* getNamespaceMap() const;
    unsigned int getPrefixId(const XMLCh* const prefix) const;
    const XMLCh* getPrefixForId(unsigned int prefId) const;

    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    void reset
    (
        const   unsigned int    emptyId
        , const unsigned int    unknownId
        , const unsigned int    xmlId
        , const unsigned int    xmlNSId
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ElemStack(const ElemStack&);
    ElemStack& operator=(const ElemStack&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void expandMap(StackElem* const toExpand);
    void expandStack();


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fEmptyNamespaceId
    //      This is the special URI id for the "" namespace, which is magic
    //      because of the xmlns="" operation.
    //
    //  fGlobalPoolId
    //      This is a special URI id that is returned when the namespace
    //      prefix is "" and no one has explicitly mapped that prefix to an
    //      explicit URI (or when they explicitly clear any such mapping,
    //      which they can also do.) And also its prefix pool id, which is
    //      stored here for fast access.
    //
    //  fPrefixPool
    //      This is the prefix pool where prefixes are hashed and given unique
    //      ids. These ids are used to track prefixes in the element stack.
    //
    //  fStack
    //  fStackCapacity
    //  fStackTop
    //      This the stack array. Its an array of pointers to StackElem
    //      structures. The capacity is the current high water mark of the
    //      stack. The top is the current top of stack (i.e. the part of it
    //      being used.)
    //
    //  fUnknownNamespaceId
    //      This is the URI id for the special URI that is assigned to any
    //      prefix which has not been mapped. This lets us keep going after
    //      issuing the error.
    //
    //  fXMLNamespaceId
    //  fXMLPoolId
    //  fXMLNSNamespaceId
    //  fXMLNSPoolId
    //      These are the URI ids for the special URIs that are assigned to
    //      the 'xml' and 'xmlns' namespaces. And also its prefix pool id,
    //      which is stored here for fast access.
    // -----------------------------------------------------------------------
    unsigned int                 fEmptyNamespaceId;
    unsigned int                 fGlobalPoolId;
    XMLStringPool                fPrefixPool;
    StackElem**                  fStack;
    unsigned int                 fStackCapacity;
    unsigned int                 fStackTop;
    unsigned int                 fUnknownNamespaceId;
    unsigned int                 fXMLNamespaceId;
    unsigned int                 fXMLPoolId;
    unsigned int                 fXMLNSNamespaceId;
    unsigned int                 fXMLNSPoolId;
    ValueVectorOf<PrefMapElem*>* fNamespaceMap;
    MemoryManager*               fMemoryManager;
};


class XMLPARSER_EXPORT WFElemStack : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Class specific data types
    //
    //  These really should be private, but some of the compilers we have to
    //  support are too dumb to deal with that.
    //
    //  PrefMapElem
    //      fURIId is the id of the URI from the validator's URI map. The
    //      fPrefId is the id of the prefix from our own prefix pool. The
    //      namespace stack consists of these elements.
    //
    //  StackElem
    //      fThisElement is the basic element decl for the current element.
    //      The fRowCapacity is how large fChildIds has grown so far.
    //      fChildCount is how many of them are valid right now.
    //
    //      The fMapCapacity is how large fMap has grown so far. fMapCount
    //      is how many of them are valid right now.
    //
    //      Note that we store the reader number we were in when we found the
    //      start tag. We'll use this at the end tag to test for unbalanced
    //      markup in entities.
    //
    //  MapModes
    //      When a prefix is mapped to a namespace id, it matters whether the
    //      QName being mapped is an attribute or name. Attributes are not
    //      affected by an sibling xmlns attributes, whereas elements are
    //      affected by its own xmlns attributes.
    // -----------------------------------------------------------------------
    struct StackElem : public XMemory
    {
        int                 fTopPrefix;        
        unsigned int        fCurrentURI;
        unsigned int        fReaderNum;
        unsigned int        fElemMaxLength;
        XMLCh*              fThisElement;
    };

    enum MapModes
    {
        Mode_Attribute
        , Mode_Element
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    WFElemStack(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~WFElemStack();


    // -----------------------------------------------------------------------
    //  Stack access
    // -----------------------------------------------------------------------
    unsigned int addLevel();
    unsigned int addLevel(const XMLCh* const toSet, const unsigned int toSetLen,
                          const unsigned int readerNum);
    const StackElem* popTop();


    // -----------------------------------------------------------------------
    //  Stack top access
    // -----------------------------------------------------------------------
    const StackElem* topElement() const;
    void setElement(const XMLCh* const toSet, const unsigned int toSetLen,
                    const unsigned int readerNum);

    void setCurrentURI(unsigned int uri);
    unsigned int getCurrentURI();

    // -----------------------------------------------------------------------
    //  Prefix map methods
    // -----------------------------------------------------------------------
    void addPrefix
    (
        const   XMLCh* const    prefixToAdd
        , const unsigned int    uriId
    );
    unsigned int mapPrefixToURI
    (
        const   XMLCh* const    prefixToMap
        , const MapModes        mode
        ,       bool&           unknown
    )   const;


    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    void reset
    (
        const   unsigned int    emptyId
        , const unsigned int    unknownId
        , const unsigned int    xmlId
        , const unsigned int    xmlNSId
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    WFElemStack(const WFElemStack&);
    WFElemStack& operator=(const WFElemStack&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void expandMap();
    void expandStack();


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fEmptyNamespaceId
    //      This is the special URI id for the "" namespace, which is magic
    //      because of the xmlns="" operation.
    //
    //  fGlobalPoolId
    //      This is a special URI id that is returned when the namespace
    //      prefix is "" and no one has explicitly mapped that prefix to an
    //      explicit URI (or when they explicitly clear any such mapping,
    //      which they can also do.) And also its prefix pool id, which is
    //      stored here for fast access.
    //
    //  fPrefixPool
    //      This is the prefix pool where prefixes are hashed and given unique
    //      ids. These ids are used to track prefixes in the element stack.
    //
    //  fStack
    //  fStackCapacity
    //  fStackTop
    //      This the stack array. Its an array of pointers to StackElem
    //      structures. The capacity is the current high water mark of the
    //      stack. The top is the current top of stack (i.e. the part of it
    //      being used.)
    //
    //  fUnknownNamespaceId
    //      This is the URI id for the special URI that is assigned to any
    //      prefix which has not been mapped. This lets us keep going after
    //      issuing the error.
    //
    //  fXMLNamespaceId
    //  fXMLPoolId
    //  fXMLNSNamespaceId
    //  fXMLNSPoolId
    //      These are the URI ids for the special URIs that are assigned to
    //      the 'xml' and 'xmlns' namespaces. And also its prefix pool id,
    //      which is stored here for fast access.
    // -----------------------------------------------------------------------
    unsigned int    fEmptyNamespaceId;
    unsigned int    fGlobalPoolId;
    unsigned int    fStackCapacity;
    unsigned int    fStackTop;
    unsigned int    fUnknownNamespaceId;
    unsigned int    fXMLNamespaceId;
    unsigned int    fXMLPoolId;
    unsigned int    fXMLNSNamespaceId;
    unsigned int    fXMLNSPoolId;
    unsigned int    fMapCapacity;
    PrefMapElem*    fMap;
    StackElem**     fStack;
    XMLStringPool   fPrefixPool;
    MemoryManager*  fMemoryManager;
};


// ---------------------------------------------------------------------------
//  ElemStack: Miscellaneous methods
// ---------------------------------------------------------------------------
inline bool ElemStack::isEmpty() const
{
    return (fStackTop == 0);
}

inline bool ElemStack::getValidationFlag()
{
    return fStack[fStackTop-1]->fValidationFlag;
}

inline void ElemStack::setValidationFlag(bool validationFlag)
{
    fStack[fStackTop-1]->fValidationFlag = validationFlag;
    return;
}

inline bool ElemStack::getCommentOrPISeen() const
{
    return fStack[fStackTop-1]->fCommentOrPISeen;
}

inline void ElemStack::setCommentOrPISeen()
{
    fStack[fStackTop-1]->fCommentOrPISeen = true;
    return;
}

inline bool ElemStack::getReferenceEscaped() const
{
    return fStack[fStackTop-1]->fReferenceEscaped;
}

inline void ElemStack::setReferenceEscaped()
{
    fStack[fStackTop-1]->fReferenceEscaped = true;
    return;
}

inline void ElemStack::setCurrentSchemaElemName(const XMLCh * const schemaElemName)
{
    unsigned int schemaElemNameLen = XMLString::stringLen(schemaElemName);
    unsigned int stackPos = fStackTop-1;
    
    if(fStack[stackPos]->fSchemaElemNameMaxLen <= schemaElemNameLen)
    {
        XMLCh *tempStr = fStack[stackPos]->fSchemaElemName;
        fStack[stackPos]->fSchemaElemNameMaxLen = schemaElemNameLen << 1;
        fStack[stackPos]->fSchemaElemName = (XMLCh *)fMemoryManager->allocate((fStack[stackPos]->fSchemaElemNameMaxLen)*sizeof(XMLCh));
        fMemoryManager->deallocate(tempStr);
    }
    XMLString::copyString(fStack[stackPos]->fSchemaElemName, schemaElemName);
}

inline XMLCh *ElemStack::getCurrentSchemaElemName()
{
    return fStack[fStackTop-1]->fSchemaElemName;
}

inline int ElemStack::getCurrentScope()
{
    return fStack[fStackTop-1]->fCurrentScope;
}

inline void ElemStack::setCurrentScope(int currentScope)
{
    fStack[fStackTop-1]->fCurrentScope = currentScope;
    return;
}

inline Grammar* ElemStack::getCurrentGrammar()
{
    return fStack[fStackTop-1]->fCurrentGrammar;
}

inline void ElemStack::setCurrentGrammar(Grammar* currentGrammar)
{
    fStack[fStackTop-1]->fCurrentGrammar = currentGrammar;
    return;
}

inline unsigned int ElemStack::getCurrentURI()
{
    return fStack[fStackTop-1]->fCurrentURI;
}

inline void ElemStack::setCurrentURI(unsigned int uri)
{
    fStack[fStackTop-1]->fCurrentURI = uri;
    return;
}

inline unsigned int ElemStack::getPrefixId(const XMLCh* const prefix) const
{
    return fPrefixPool.getId(prefix);
}

inline const XMLCh* ElemStack::getPrefixForId(unsigned int prefId) const
{
    return fPrefixPool.getValueForId(prefId);
}

inline void ElemStack::setPrefixColonPos(int colonPos)
{
    fStack[fStackTop-1]->fPrefixColonPos = colonPos;
}
 
inline int ElemStack::getPrefixColonPos() const {
    return fStack[fStackTop-1]->fPrefixColonPos;
}

// ---------------------------------------------------------------------------
//  WFElemStack: Miscellaneous methods
// ---------------------------------------------------------------------------
inline bool WFElemStack::isEmpty() const
{
    return (fStackTop == 0);
}

inline unsigned int WFElemStack::getCurrentURI()
{
    return fStack[fStackTop-1]->fCurrentURI;
}

inline void WFElemStack::setCurrentURI(unsigned int uri)
{
    fStack[fStackTop-1]->fCurrentURI = uri;
    return;
}


XERCES_CPP_NAMESPACE_END

#endif
