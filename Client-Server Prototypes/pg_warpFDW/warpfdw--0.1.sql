/*---------------------------------------------------------------
 * foreign data-wrapper for postgres
 *
 * Original author: Joel Ziegler <cody14@freenet.de>
 *---------------------------------------------------------------
 */


CREATE FUNCTION warpfdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
/*
CREATE FUNCTION warpfdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
*/
CREATE FOREIGN DATA WRAPPER warpfdw
  HANDLER warpfdw_handler
/*  VALIDATOR warpfdw_validator*/;