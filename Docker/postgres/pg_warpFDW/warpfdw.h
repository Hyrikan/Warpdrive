//
// Created by joel on 13.06.22.
//

#ifndef FDW_PROTOTYPE_warpfdw_H
#define FDW_PROTOTYPE_warpfdw_H


#include <warp_client.h>


#include "postgres.h"
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "nodes/nodes.h"
#include "optimizer/pathnode.h"
#include "optimizer/planmain.h"
#include "optimizer/restrictinfo.h"
#include "nodes/pg_list.h"
#include <string.h>

#include "funcapi.h"
#include "access/reloptions.h"
#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"
#include "catalog/pg_user_mapping.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/explain.h"
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "utils/memutils.h"
#include "utils/builtins.h"
#include "utils/relcache.h"
#include "utils/date.h"
#include "utils/numeric.h"
#include "storage/lock.h"
#include "miscadmin.h"
#include "mb/pg_wchar.h"
#include "optimizer/cost.h"
#include "storage/fd.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "nodes/nodes.h"
#include "nodes/makefuncs.h"
#include "nodes/pg_list.h"
#include "parser/parse_type.h"
#include "fmgr.h"

#include "optimizer/pathnode.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/planmain.h"

#include "access/tupdesc.h"


#endif //FDW_PROTOTYPE_warpfdw_H
