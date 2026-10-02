#pragma once

#include <string>

#include "advaiknowledgebaseserviceglue.h"

extern void knowledgeBaseserviceSyncResponse(const std::string& method, GObject* delegate,
                                             GDBusMethodInvocation* invocation, const std::string& output);

extern void knowledgeBaseserviceAsyncResponse(const std::string& method, GObject* delegate,
                                              GDBusMethodInvocation* invocation, const std::string& output);

extern void knowledgeBaseserviceAsyncFlowResponse(const std::string& method, GDBusConnection* connection,
                                                  GObject* delegate, GDBusMethodInvocation* invocation,
                                                  GVariant* output, bool end);