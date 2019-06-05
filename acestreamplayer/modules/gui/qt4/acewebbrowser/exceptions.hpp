#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "acewebbrowser/defines.hpp"

namespace AceWebBrowser
{

class Exception
{
public:
    Exception() : mDescription("Exception description not defined") {}
    Exception(const QString& message) : mDescription(message) {}
    virtual ~Exception() {}

    const QString& message() const {
        return mDescription;
    }
private:
    QString mDescription;
};

class BrowserException : public Exception
{
public:
    BrowserException() : Exception() {}
    BrowserException(const QString& message)
        : Exception(message)
        , mType(BTYPE_UNDEFINED)
        , mId("")
    {}
    BrowserException(const QString& message, BrowserType type, const QString& id)
        : Exception(message)
        , mType(type)
        , mId(id)
    {}
    virtual ~BrowserException() {}

    const BrowserType type() const {
        return mType;
    }
    const QString& id() const {
        return mId;
    }

private:
    BrowserType mType;
    QString mId;
};

}

#endif // EXCEPTIONS_H
