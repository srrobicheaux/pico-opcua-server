#ifndef OPC_HANDLERS_H
#define OPC_HANDLERS_H

#include "open62541/server.h"

/**
 * @brief Registers all folders, variables, and properties into the OPC UA Server.
 * @param server Pointer to the active UA_Server instance.
 */
void add_variables(UA_Server *server);

#endif /* OPC_HANDLERS_H */