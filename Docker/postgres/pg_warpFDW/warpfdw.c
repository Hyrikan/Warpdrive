/*-------------------------------------------------------------------------
 * warpfdw.c
 *	foreign data wrapper for postgres. Using warpdrive and only able to receive postgres data as row oriented
 *	and clickhouse data as column oriented.
 * made by Joel Ziegler
 *-------------------------------------------------------------------------
 */
/* Debug mode flag */
//#define DEBUG

/* Macro to make conditional DEBUG more terse
 * Usage: elog(String); output can be found in console*/
#ifdef DEBUG
#define elog_debug(...) elog(WARNING, __VA_ARGS__)
#else
#define elog_debug(...) ((void) 0)
#endif

#include "warpfdw.h"

PG_MODULE_MAGIC;

/*
 * SQL functions
 */
PG_FUNCTION_INFO_V1(warpfdw_handler);
PG_FUNCTION_INFO_V1(warpfdw_validator);

/*
 *  Extension initialization functions
 */
extern void _PG_init(void);
extern void _PG_fini(void);

/*
 * FDW callback routines
 */
static void warpfdwBeginForeignScan(ForeignScanState *node, int eflags);
static TupleTableSlot *warpfdwIterateForeignScan(ForeignScanState *node);
static void warpfdwReScanForeignScan(ForeignScanState *node);
static void warpfdwEndForeignScan(ForeignScanState *node);
static void warpfdwGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static void warpfdwGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid);
static ForeignScan* warpfdwGetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid, ForeignPath *best_path, List *tlist, List *scan_clauses, Plan *outer_plan);

/*
 * Helper functions
 */
static void
warpfdwGetOptions(Oid foreigntableid, int *maxheapsize, char **username, char **password, char **host, int *port, char **query, char **table, bool *rowFormat);
static HeapTuple warpfdwFetchOneRow(TupleDesc tDesc);
char *getStrFromPGNumeric(u_int16_t *numvar);
char *getNextBufferVal(size_t length);

/*
 * Helper variables
 */
size_t fetchedDataLength;
size_t fetchedDataUsed;
unsigned char *fetchedDataBuf;
bool transferFormat = false;


void _PG_init(void){
    elog_debug("Initialization of FDW!");
}

// Gets NEVER called!! (May change in postgres future)
void _PG_fini(void){
    elog_debug("deinitialization got called");
}


/*
 * The handler function returns function pointers to all the important fdw functions
 *
 */
Datum warpfdw_handler(PG_FUNCTION_ARGS){
	FdwRoutine *fdwroutine = makeNode(FdwRoutine);

	fdwroutine->GetForeignRelSize = warpfdwGetForeignRelSize;
	fdwroutine->GetForeignPaths = warpfdwGetForeignPaths;
	fdwroutine->GetForeignPlan = warpfdwGetForeignPlan;
	fdwroutine->BeginForeignScan = warpfdwBeginForeignScan;
	fdwroutine->IterateForeignScan = warpfdwIterateForeignScan;
	fdwroutine->ReScanForeignScan = warpfdwReScanForeignScan;
	fdwroutine->EndForeignScan = warpfdwEndForeignScan;

	elog_debug("%s",__func__);

	PG_RETURN_POINTER(fdwroutine);
}


/*
 * This function should update baserel->rows to be the expected number of rows returned by the table scan,
 * after accounting for the filtering done by the restriction quals.
 * The initial value of baserel->rows is just a constant default estimate, which should be replaced if at all possible.
 * The function may also choose to update baserel->width if it can compute a better estimate of the average result row width.
 * (The initial value is based on column data types and on column average-width values measured by the last ANALYZE.)
 * Also, this function may update baserel->tuples if it can compute a better estimate of the foreign table's total row count.
 * (The initial value is from pg_class.reltuples which represents the total row count seen by the last ANALYZE.)
 */
static void warpfdwGetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid){
    baserel->rows = 3;
    elog_debug("%s",__func__);
}

/*
 * Fetches values from OPTIONS in Foreign Server and Table registration.
 */
static void
warpfdwGetOptions(Oid foreigntableid, int *maxheapsize, char **username, char **password, char **host, int *port, char **query, char **table, bool *rowFormat)
{
    elog_debug("%s",__func__);
    ForeignTable	*f_table;
    ForeignServer	*f_server;
    UserMapping	*f_mapping;
    List		*options;
    ListCell	*lc;

    /*
     * Extract options from FDW objects.
     */
    f_table = GetForeignTable(foreigntableid);
    f_server = GetForeignServer(f_table->serverid);
    f_mapping = GetUserMapping(GetUserId(), f_table->serverid);

    options = NIL;
    options = list_concat(options, f_table->options);
    options = list_concat(options, f_server->options);
    options = list_concat(options, f_mapping->options);

    /* Loop through the options, and get the server/port */
    foreach(lc, options)
    {
        DefElem *def = (DefElem *) lfirst(lc);

        if (strcmp(def->defname, "user") == 0)
        {
            *username = defGetString(def);
            elog_debug("Got username with value: %s", *username);
        }

        if (strcmp(def->defname, "maxheapsize") == 0)
        {
            *maxheapsize = defGetInt32(def);
            elog_debug("Got maxheapsize with value: %d", *maxheapsize);
        }

        if (strcmp(def->defname, "password") == 0)
        {
            *password = defGetString(def);
            elog_debug("Got password with value: %s", *password);
        }

        if (strcmp(def->defname, "query") == 0)
        {
            *query = defGetString(def);
            elog_debug("Got query with value: %s", *query);
        }

        if (strcmp(def->defname, "table") == 0)
        {
            *table = defGetString(def);
            elog_debug("Got table with value: %s", *table);
        }

        if (strcmp(def->defname, "port") == 0)
        {
            *port = atoi(defGetString(def));
            elog_debug("Got port with value: %d", *port);
        }

        if (strcmp(def->defname, "receiveinrow") == 0)
        {
            *rowFormat = strcmp(defGetString(def),"True")==0 ? true : false;
            elog_debug("Got row format with value: %s", *rowFormat ? "True" : "False");
        }

        if (strcmp(def->defname, "host") == 0)
        {
            *host = defGetString(def);
            elog_debug("Got host with value: %s", *host);
        }
    }
}

/*
 * This function must generate at least one access path (ForeignPath node) for a scan on the foreign table
 * and must call add_path to add each such path to baserel->pathlist.
 * It's recommended to use create_foreignscan_path to build the ForeignPath nodes.
 * The function can generate multiple access paths, e.g., a path which has valid pathkeys to represent a pre-sorted result.
 * Each access path must contain cost estimates,
 * and can contain any FDW-private information that is needed to identify the specific scan method intended.
 */
static void warpfdwGetForeignPaths(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid){
    int dummycounter;

    // dummy cost values
    Cost startup_cost = 25;
    Cost total_cost = 25;
    elog_debug("%s",__func__);
    // dummy counter in fdw_private field
    dummycounter = 3;
    List *fdw_private = list_make1_int(dummycounter);

    /*
  * add_path
  *    Consider a potential implementation path for the specified parent rel,
  *    and add it to the rel's pathlist if it is worthy of consideration.
  *    A path is worthy if it has a better sort order (better pathkeys) or
  *    cheaper cost (on either dimension), or generates fewer rows, than any
  *    existing path that has the same or superset parameterization rels.
  *    We also consider parallel-safe paths more worthy than others.
  *
  *    We also remove from the rel's pathlist any old paths that are dominated
  *    by new_path --- that is, new_path is cheaper, at least as well ordered,
  *    generates no more rows, requires no outer rels not required by the old
  *    path, and is no less parallel-safe.
  *
  *    In most cases, a path with a superset parameterization will generate
  *    fewer rows (since it has more join clauses to apply), so that those two
  *    figures of merit move in opposite directions; this means that a path of
  *    one parameterization can seldom dominate a path of another.  But such
  *    cases do arise, so we make the full set of checks anyway.
  *
  *    There are two policy decisions embedded in this function, along with
  *    its sibling add_path_precheck.  First, we treat all parameterized paths
  *    as having NIL pathkeys, so that they cannot win comparisons on the
  *    basis of sort order.  This is to reduce the number of parameterized
  *    paths that are kept; see discussion in src/backend/optimizer/README.
  *
  *    Second, we only consider cheap startup cost to be interesting if
  *    parent_rel->consider_startup is true for an unparameterized path, or
  *    parent_rel->consider_param_startup is true for a parameterized one.
  *    Again, this allows discarding useless paths sooner.
  *
  *    The pathlist is kept sorted by total_cost, with cheaper paths
  *    at the front.  Within this routine, that's simply a speed hack:
  *    doing it that way makes it more likely that we will reject an inferior
  *    path after a few comparisons, rather than many comparisons.
  *    However, add_path_precheck relies on this ordering to exit early
  *    when possible.
  *
  *    NOTE: discarded Path objects are immediately pfree'd to reduce planner
  *    memory consumption.  We dare not try to free the substructure of a Path,
  *    since much of it may be shared with other Paths or the query tree itself;
  *    but just recycling discarded Path nodes is a very useful savings in
  *    a large join tree.  We can recycle the List nodes of pathlist, too.
  *
  *    As noted in optimizer/README, deleting a previously-accepted Path is
  *    safe because we know that Paths of this rel cannot yet be referenced
  *    from any other rel, such as a higher-level join.  However, in some cases
  *    it is possible that a Path is referenced by another Path for its own
  *    rel; we must not delete such a Path, even if it is dominated by the new
  *    Path.  Currently this occurs only for IndexPath objects, which may be
  *    referenced as children of BitmapHeapPaths as well as being paths in
  *    their own right.  Hence, we don't pfree IndexPaths when rejecting them.
  *
  * 'parent_rel' is the relation entry to which the path corresponds.
  * 'new_path' is a potential path for parent_rel.
  *
  * Returns nothing, but modifies parent_rel->pathlist.
  */
    add_path(baserel,
             /*
 * create_foreignscan_path
 *    Creates a path corresponding to a scan of a foreign base table,
 *    returning the pathnode.
 *
 * This function is never called from core Postgres; rather, it's expected
 * to be called by the GetForeignPaths function of a foreign data wrapper.
 * We make the FDW supply all fields of the path, since we do not have any way
 * to calculate them in core.  However, there is a usually-sane default for
 * the pathtarget (rel->reltarget), so we let a NULL for "target" select that.
 */
             (Path *) create_foreignscan_path(root, baserel, NULL, baserel->rows, startup_cost, total_cost,
                                     NIL, NULL, NULL, fdw_private));


    /*
    elog_debug("Check, whether i can get my dummycounter back.");
    if(linitial_int(fdw_private) == 3) elog_debug("Yes i get it back!");
     */
}

/*
 * Create a ForeignScan plan node from the selected foreign access path. This is called at the end of query planning.
 * The parameters are as for GetForeignRelSize, plus the selected ForeignPath
 * (previously produced by GetForeignPaths, GetForeignJoinPaths, or GetForeignUpperPaths),
 * the target list to be emitted by the plan node, the restriction clauses to be enforced by the plan node,
 * and the outer subplan of the ForeignScan, which is used for rechecks performed by RecheckForeignScan.
 * (If the path is for a join rather than a base relation, foreigntableid is InvalidOid.)
 *
 * This function must create and return a ForeignScan plan node;
 * it's recommended to use make_foreignscan to build the ForeignScan node.
 */
static ForeignScan* warpfdwGetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid, ForeignPath *best_path, List *tlist, List *scan_clauses, Plan *outer_plan){
    List *fdw_private;

    elog_debug("%s",__func__);
    // Retain fdw_private information.
    fdw_private = best_path->fdw_private;

    // Just copied!!
    Index scan_relid = baserel->relid;
    scan_clauses = extract_actual_clauses(scan_clauses, false);

    /* ----------------
 *      ForeignScan node
 *
 * fdw_exprs and fdw_private are both under the control of the foreign-data
 * wrapper, but fdw_exprs is presumed to contain expression trees and will
 * be post-processed accordingly by the planner; fdw_private won't be.
 * Note that everything in both lists must be copiable by copyObject().
 * One way to store an arbitrary blob of bytes is to represent it as a bytea
 * Const.  Usually, though, you'll be better off choosing a representation
 * that can be dumped usefully by nodeToString().
 *
 * fdw_scan_tlist is a targetlist describing the contents of the scan tuple
 * returned by the FDW; it can be NIL if the scan tuple matches the declared
 * rowtype of the foreign table, which is the normal case for a simple foreign
 * table scan.  (If the plan node represents a foreign join, fdw_scan_tlist
 * is required since there is no rowtype available from the system catalogs.)
 * When fdw_scan_tlist is provided, Vars in the node's tlist and quals must
 * have varno INDEX_VAR, and their varattnos correspond to resnos in the
 * fdw_scan_tlist (which are also column numbers in the actual scan tuple).
 * fdw_scan_tlist is never actually executed; it just holds expression trees
 * describing what is in the scan tuple's columns.
 *
 * fdw_recheck_quals should contain any quals which the core system passed to
 * the FDW but which were not added to scan.plan.qual; that is, it should
 * contain the quals being checked remotely.  This is needed for correct
 * behavior during EvalPlanQual rechecks.
 *
 * When the plan node represents a foreign join, scan.scanrelid is zero and
 * fs_relids must be consulted to identify the join relation.  (fs_relids
 * is valid for simple scans as well, but will always match scan.scanrelid.)
 *
 * If the FDW's PlanDirectModify() callback decides to repurpose a ForeignScan
 * node to perform the UPDATE or DELETE operation directly in the remote
 * server, it sets 'operation' and 'resultRelation' to identify the operation
 * type and target relation.  Note that these fields are only set if the
 * modification is performed *fully* remotely; otherwise, the modification is
 * driven by a local ModifyTable node and 'operation' is left to CMD_SELECT.
 * ----------------
 */
    return make_foreignscan(tlist, scan_clauses, scan_relid, NIL, fdw_private, NIL, NIL, NULL);
}

/*
 * Begin executing a foreign scan. This is called during executor startup.
 * It should perform any initialization needed before the scan can start,
 * but not start executing the actual scan (that should be done upon the first call to IterateForeignScan).
 * The ForeignScanState node has already been created, but its fdw_state field is still NULL.
 * Information about the table to scan is accessible through the ForeignScanState node
 * (in particular, from the underlying ForeignScan plan node, which contains any FDW-private information provided by GetForeignPlan).
 * eflags contains flag bits describing the executor's operating mode for this plan node.
 */
static void warpfdwBeginForeignScan(ForeignScanState *node, int eflags){
    char		*svr_username;
    char		*svr_password;
    char 		*svr_query;
    char 		*svr_table;
    char        *svr_host;
    int 		svr_maxheapsize;
    int         svr_port;
    int         ret = 0;

    //ForeignScan *plan = (ForeignScan *) node->ss.ps.plan;
    elog_debug("%s",__func__);

    // Query acquisition from options... weird ^^
    svr_username = NULL;
    svr_password = NULL;
    svr_port = 0;
    svr_query = NULL;
    svr_table = NULL;
    svr_host  = NULL;
    svr_maxheapsize = 0;

    warpfdwGetOptions(RelationGetRelid(node->ss.ss_currentRelation),
                       &svr_maxheapsize,
                       &svr_username,
                       &svr_password,
                       &svr_host,
                       &svr_port,
                       &svr_query,
                       &svr_table,
                       &transferFormat
    );

    // start consulting the warpserver via warpclient
    ret = warpclient_queryServer(svr_host, svr_port, 0, transferFormat,svr_query);
    if(ret != 0){
        ereport(ERROR, (errcode(1)), errmsg("Can't connect to warpserver"));
    }
    fetchedDataLength = 0;
    fetchedDataUsed = 0;
    fetchedDataBuf = NULL;
}

/*
 * Fetch one row from the foreign source, returning it in a tuple table slot
 * (the node's ScanTupleSlot should be used for this purpose). Return NULL if no more rows are available.
 * The tuple table slot infrastructure allows either a physical or virtual tuple to be returned;
 * in most cases the latter choice is preferable from a performance standpoint.
 * Note that this is called in a short-lived memory context that will be reset between invocations.
 * Create a memory context in BeginForeignScan if you need longer-lived storage, or use the es_query_cxt of the node's EState.
 */
static TupleTableSlot *warpfdwIterateForeignScan(ForeignScanState *node){
    elog_debug("%s",__func__);

    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    TupleDesc tDescFromNode = node->ss.ss_currentRelation->rd_att;
    HeapTuple tuple;

    elog_debug("Fetch next heap");

    /*
    ForeignScan *plan = (ForeignScan *) node->ss.ps.plan;
    int dummycounter =  linitial_int(plan->fdw_private);
    if(dummycounter >0) {
        Datum *columns = (Datum *) palloc0(sizeof(Datum) * 3);
        bool *isnull = (bool *) palloc(sizeof(bool) * 3);
        char *testtext = (char *) palloc0(25);
        memcpy(testtext, "Test", 5);
        isnull[0] = false;
        isnull[1] = false;
        isnull[2] = false;
        columns[0] = Int32GetDatum(24);
        columns[1] = CStringGetTextDatum(testtext);
        memcpy(testtext, "Hallo", 6);
        columns[2] = CStringGetTextDatum(testtext);
        tuple = heap_form_tuple(tDescFromNode, columns, isnull);

        list_head(plan->fdw_private)->data.int_value--;

        ExecClearTuple(slot);
        elog_debug("Store tuple in TupleTableSlot");
        ExecStoreHeapTuple(tuple, slot, false);
        return slot;
    }
    else return NULL;
    */

    tuple = warpfdwFetchOneRow(tDescFromNode);
    if(tuple == NULL) return NULL;
    ExecClearTuple(slot);
    elog_debug("Store tuple in TupleTableSlot");
    ExecStoreHeapTuple(tuple, slot, false);
    return slot;


/*

    // dummy table
    ForeignScan *plan = (ForeignScan *) node->ss.ps.plan;
    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    Datum *values;
    //AttInMetadata *attin;
    char **cstringvalues;
    //TupleDesc tDesc;
    TupleDesc tDescFromNode = node->ss.ss_currentRelation->rd_att;
    HeapTuple tuple;
    //List *colaliases;
    int dummycounter;
    //Oid oid;
    bool *isnull = palloc0(sizeof(bool)*3);
    isnull[0]=false;
    isnull[1]=false;
    isnull[2]=false;

    // dummy data array as Datum *
    values = (Datum *) palloc0(sizeof(Datum)*3);
    // as plain cstrings
    cstringvalues = (char **) palloc0(sizeof(char *)*3);
    for(int i = 0; i < 3; i++){
        cstringvalues[i] = (char *) palloc0(sizeof(char) * 10);
    }


    dummycounter =  linitial_int(plan->fdw_private);


 //tuple made from datum *values
    if( dummycounter > 0){
        if(dummycounter == 1){
            values[0] = Int32GetDatum(3);
            strcpy(cstringvalues[0],"Charly");
            values[1] = CStringGetTextDatum(cstringvalues[0]);
            strcpy(cstringvalues[1],"Vogler");
            values[2] = CStringGetTextDatum(cstringvalues[1]);
        }
        else if(dummycounter == 2){
            values[0] = Int32GetDatum(2);
            values[1] = CStringGetTextDatum("Lara");
            strcpy(cstringvalues[1],"Jurisch");
            values[2] = CStringGetTextDatum(cstringvalues[1]);
        }
        else if(dummycounter == 3){
            values[0] = Int32GetDatum(1);
            strcpy(cstringvalues[0],"Joel");
            values[1] = CStringGetTextDatum(cstringvalues[0]);
            strcpy(cstringvalues[1],"Ziegler");
            values[2] = CStringGetTextDatum(cstringvalues[1]);
        }
        list_head(plan->fdw_private)->data.int_value--;

        ExecClearTuple(slot);

        tuple = heap_form_tuple(tDescFromNode, values, isnull);
        ExecStoreHeapTuple(tuple, slot, false);
        return slot;

    }
    */


/*
 * tuple made from cstrings values (char **values)

    if( dummycounter > 0){
        if(dummycounter == 1){
            strcpy(cstringvalues[0], "3");
            strcpy(cstringvalues[1], "Charly");
            strcpy(cstringvalues[2], "Vogler");
            }
        else if(dummycounter == 2){
            strcpy(cstringvalues[0], "2");
            strcpy(cstringvalues[1], "Lara");
            strcpy(cstringvalues[2], "Jurisch");
        }
        else if(dummycounter == 3){
            strcpy(cstringvalues[0], "1");
            strcpy(cstringvalues[1], "Joel");
            strcpy(cstringvalues[2], "Ziegler");
        }
        list_head(plan->fdw_private)->data.int_value--;

        ExecClearTuple(slot);

        attin = TupleDescGetAttInMetadata(tDescFromNode);
        elog_debug("while tuple creation");
        tuple = BuildTupleFromCStrings(attin,cstringvalues);
        elog_debug("while tuple storing");
        ExecStoreHeapTuple(tuple, slot, false);
        elog_debug("Tuple returned");
        return slot;

    }
    else return NULL;*/
}

/*
 * Restart the scan from the beginning. Note that any parameters the scan depends on may have changed value,
 * so the new scan does not necessarily return exactly the same rows.
 */
static void warpfdwReScanForeignScan(ForeignScanState *node){elog_debug("%s",__func__);}

/*
 * End the scan and release resources. It is normally not important to release palloc'd memory,
 * but for example open files and connections to remote servers should be cleaned up.
 */
static void warpfdwEndForeignScan(ForeignScanState *node){
    ForeignScan *plan = (ForeignScan *) node->ss.ps.plan;
    elog_debug("%s", __func__);
    list_free(plan->fdw_private);

    // release warpclient ressources
    int ret = warpclient_cleanup();
    elog_debug("Released warp_client ressources. Status: %d", ret);
}

/*
 * Fetches one row and assumes row format in the received data buffer. SPECIALIZED FOR pg1_sf1_region!
 */
static HeapTuple warpfdwFetchOneRowFromRowRegion(TupleDesc tDesc){
    elog_debug("Fetch next row from warpclient data buffer...");
    Datum *columns = (Datum *) palloc0(sizeof(Datum)*16);
    bool *isnull = (bool *) palloc0(sizeof(bool)*16);


    int nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    char *nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    int id = ntohl(*(int *)nextvalue);
    elog_debug("Int value before conversion: %d", id);
    columns[0] = Int32GetDatum(id);
    isnull[0] = false;
    free(nextvalue);

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    elog_debug("String value before conversion: %s", nextvalue);
    columns[1] = CStringGetTextDatum(nextvalue);
    isnull[1] = false;

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    elog_debug("String value before conversion: %s", nextvalue);
    columns[2] = CStringGetTextDatum(nextvalue);
    isnull[2] = false;


    HeapTuple result = heap_form_tuple(tDesc, columns, isnull);
    return result;
}


/*
 * Converts a postgres numeric into a string representation
 */
char *getStrFromPGNumeric(u_int16_t *numvar){
    //u_int16_t ndigits = ntohs(numvar[0]); // how many u_int16_t at numvar[4]
    int16_t dscale = ntohs(numvar[3]); // how many char digits after decimal point
    int16_t weight = ntohs(numvar[1])+1; // weight+1 is how many u_int16_t from numvar[4] are before decimal point. here weight already gets +1 at initialization.
    char *result = (char *)malloc(sizeof(char)*(weight+dscale)+1+1+2); // +1+1 -> '\0' and '.'
    char *copyStr = (char *) malloc(sizeof (char)*5);
    int strindex = 0;
    int numvarindex = 0;
    while(weight>0){
        sprintf(copyStr, "%d", ntohs(numvar[numvarindex+4]));
        sprintf(&(result[strindex]), "%s", copyStr);
        strindex += strlen(copyStr);
        numvarindex++;
        weight--;
    }
    sprintf(&(result[strindex]), ".");
    strindex++;
    while(dscale>0){
        sprintf(copyStr, "%d", ntohs(numvar[numvarindex+4]));
        dscale -= strlen(copyStr);
        sprintf(&(result[strindex]), "%s", copyStr);
        strindex += strlen(copyStr);
        numvarindex++;
    }
    result[strindex+dscale] = '\0';
    free(copyStr);
    return result;
}

/*
 * copies next value from warpbuffer into a small buffer and returns it. IF reaching end of buffer, fetches new one and continue copy..
 */
char *getNextBufferVal(size_t length){
    size_t spaceLeft = fetchedDataLength-fetchedDataUsed;
    char *result = (char *) calloc(length+1, 1);
    // is the whole value in our current buffer? just return it..
    if(spaceLeft >= length){
        memcpy(result, (fetchedDataBuf+fetchedDataUsed), length);
        fetchedDataUsed += length;
        return result;
    }
    elog_debug("=============================  VALUE OVER BUFFER BOUNDARIES ============================");
    // else we need to put it together from this buffer and the next, we need to fetch in between.
    // copy first part from rest of buffer
    memcpy(result, (fetchedDataBuf+fetchedDataUsed), spaceLeft);
    // get new buffer
    fetchedDataBuf = warpclient_getData(&fetchedDataLength);
    if(fetchedDataBuf == NULL){
        elog_debug("================================ ? NULL IN VALUE GET FUNCTION IS WRONG!!! ================================");
        free(result);
        result = NULL;
        return result;
    }
    elog_debug("Size of next buffer: %lu", fetchedDataLength);
    fetchedDataUsed = 0;
    // copy part two from new buffer
    memcpy((result+spaceLeft), (fetchedDataBuf+fetchedDataUsed), (length-spaceLeft));
    fetchedDataUsed += (length - spaceLeft);
    return result;
}

/*
 * Fetches one row and assumes row format in the received data buffer. SPECIALIZED FOR pg1_sfX_lineitem! TODO handle numerics and dates
 */
static HeapTuple warpfdwFetchOneRowFromRowLineitem(TupleDesc tDesc){
    elog_debug("Fetch next row from warpclient data buffer...");
    Datum *columns = (Datum *) palloc0(sizeof(Datum)*9);
    bool *isnull = (bool *) palloc0(sizeof(bool)*9);
    int i = 0;
    int nextValLength;
    char *nextvalue;



    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    int id = __bswap_64((*(u_int64_t *)nextvalue));
    elog_debug("l_orderkey: %d", id);
    columns[i] = Int64GetDatum(id);
    isnull[i++] = false;
    free(nextvalue);

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    int id2 = __bswap_32((*(u_int32_t *)nextvalue));
    elog_debug("l_partkey: %d", id2);
    columns[i] = Int32GetDatum(id2);
    isnull[i++] = false;
    free(nextvalue);

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    int id3 = __bswap_32((*(u_int32_t *)nextvalue));
    elog_debug("l_suppkey: %d", id3);
    columns[i] = Int32GetDatum(id3);
    isnull[i++] = false;
    free(nextvalue);

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    int id4 = __bswap_32((*(u_int32_t *)nextvalue));
    elog_debug("l_linenumber: %d", id4);
    columns[i] = Int32GetDatum(id4);
    isnull[i++] = false;
    free(nextvalue);

    /*nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    char *printstr = getStrFromPGNumeric(((u_int16_t *)(fetchedDataBuf+fetchedDataUsed)));
    fetchedDataUsed += nextValLength;
    elog_debug("l_quantity: %.*s", nextValLength, printstr);
    *//*u_int16_t *id5 = (u_int16_t *)nextvalue;
    columns[i] = DirectFunctionCall3(numeric_recv,PointerGetDatum(id5),ObjectIdGetDatum(InvalidOid),Int32GetDatum(0));
    columns[i] =
    isnull[i++] = false;*//*
    free(nextvalue);
    free(printstr);

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    printstr = getStrFromPGNumeric(((u_int16_t *)(fetchedDataBuf+fetchedDataUsed)));
    fetchedDataUsed += nextValLength;
    elog_debug("l_extendedprice: %.*s", nextValLength, printstr);
    *//*u_int16_t *id6 = (u_int16_t *)nextvalue;
    columns[i] = DirectFunctionCall3(numeric_recv,PointerGetDatum(id6),ObjectIdGetDatum(InvalidOid),Int32GetDatum(0));
    isnull[i++] = false;*//*
    free(nextvalue);
    free(printstr);

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    printstr = getStrFromPGNumeric(((u_int16_t *)(fetchedDataBuf+fetchedDataUsed)));
    fetchedDataUsed += nextValLength;
    elog_debug("l_discount: %.*s", nextValLength, printstr);
    *//*u_int16_t *id7 = (u_int16_t *)nextvalue;
    columns[i] = DirectFunctionCall3(numeric_recv,PointerGetDatum(id7),ObjectIdGetDatum(InvalidOid),Int32GetDatum(0));
    isnull[i++] = false;*//*
    free(nextvalue);
    free(printstr);

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    printstr = getStrFromPGNumeric(((u_int16_t *)(fetchedDataBuf+fetchedDataUsed)));
    fetchedDataUsed += nextValLength;
    elog_debug("l_tax: %.*s", nextValLength, printstr);
    *//*u_int16_t *id8 = (u_int16_t *)nextvalue;
    columns[i] = DirectFunctionCall3(numeric_recv,PointerGetDatum(id8),ObjectIdGetDatum(InvalidOid),Int32GetDatum(0));
    isnull[i++] = false;*//*
    free(nextvalue);
    free(printstr);*/

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    elog_debug("l_returnflag: %.*s", nextValLength, nextvalue);
    columns[i] = CStringGetTextDatum(nextvalue);
    isnull[i++] = false;

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    elog_debug("l_linestatus: %.*s", nextValLength, nextvalue);
    columns[i] = CStringGetTextDatum(nextvalue);
    isnull[i++] = false;

    /*nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    *//*elog_debug("l_shipdate: %.*s", nextValLength, nextvalue);
    columns[i] = DirectFunctionCall1(date_recv,PointerGetDatum(nextvalue));
    isnull[i++] = false;*//*

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    *//*elog_debug("l_commitdate: %.*s", nextValLength, nextvalue);
    columns[i] = DirectFunctionCall1(date_recv,PointerGetDatum(nextvalue));
    isnull[i++] = false;*//*

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) calloc(nextValLength, 1);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    *//*elog_debug("l_receiptdate: %.*s", nextValLength, nextvalue);
    columns[i] = DirectFunctionCall1(date_recv,PointerGetDatum(nextvalue));
    isnull[i++] = false;*/

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    elog_debug("l_shipinstruct: %.*s", nextValLength, nextvalue);
    columns[i] = CStringGetTextDatum(nextvalue);
    isnull[i++] = false;

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    elog_debug("l_shipmode: %.*s", nextValLength, nextvalue);
    columns[i] = CStringGetTextDatum(nextvalue);
    isnull[i++] = false;

    nextValLength = * (int *) getNextBufferVal(4);
    nextvalue = getNextBufferVal(nextValLength);
    elog_debug("Vallen: %d ends at index %ld", nextValLength, fetchedDataUsed);
    elog_debug("l_comment: %.*s", nextValLength, nextvalue);
    columns[i] = CStringGetTextDatum(nextvalue);
    isnull[i++] = false;


    HeapTuple result = heap_form_tuple(tDesc, columns, isnull);
    return result;
}

/*
 * Fetches one row and assumes column format in the received data buffer. //TODO Make specialized for ch column data... >.>
 */
static HeapTuple warpfdwFetchOneRowFromColumn(TupleDesc tDesc){
    Datum *columns = palloc0(sizeof(Datum)*3);
    bool *isnull = palloc0(sizeof(bool)*3);

    char *nextvalue;

    int nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) malloc(nextValLength);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    int32_t id = ntohl(*(int32_t*)nextvalue);
    elog_debug("id: %d", id);
    columns[0] = Int32GetDatum(id);
    isnull[0] = false;
    free(nextvalue);

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) malloc(nextValLength);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    elog_debug("region: %.*s", nextValLength, nextvalue);
    columns[1] = CStringGetTextDatum(nextvalue);
    isnull[1] = false;

    nextValLength = * (int *)(fetchedDataBuf+fetchedDataUsed);
    nextvalue = (char *) malloc(nextValLength);
    fetchedDataUsed += sizeof(int);
    elog_debug("Vallen: %d at index %ld", nextValLength, fetchedDataUsed);
    memcpy(nextvalue, (fetchedDataBuf+fetchedDataUsed), nextValLength);
    fetchedDataUsed += nextValLength;
    elog_debug("comment: %.*s", nextValLength, nextvalue);
    columns[2] = CStringGetTextDatum(nextvalue);
    isnull[2] = false;

    HeapTuple result = heap_form_tuple(tDesc, columns, isnull);
    return result;
}

/*
 * Returns one tuple as datum *. If no data available, fetches more via warp_client.
 * Only for test use. No real function for now.
 */
static HeapTuple warpfdwFetchOneRow(TupleDesc tDesc){
    HeapTuple result;
    // no data left in buffer? fetch more.
    if(fetchedDataLength <= fetchedDataUsed){
        elog_debug("=============================  Fetch next buffer  ============================");
        fetchedDataBuf = warpclient_getData(&fetchedDataLength);
        if(fetchedDataBuf == NULL){
            result = NULL;
            return result;
        }
        elog_debug("Size of next buffer: %lu", fetchedDataLength);
        fetchedDataUsed = 0;
    }



    if(transferFormat) result = warpfdwFetchOneRowFromRowLineitem(tDesc);
    else result = warpfdwFetchOneRowFromColumn(tDesc);

    return result;
}






















