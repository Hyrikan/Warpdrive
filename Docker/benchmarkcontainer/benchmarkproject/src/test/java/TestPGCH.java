import clickhouse.ClickhouseInteractor;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import postgres.PostgresInteractor;

import javax.imageio.IIOException;
import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.fail;

public class TestPGCH {
    static ClickhouseInteractor click1;
    static PostgresInteractor pg1;
    static int lineitemsf1_lines = 6001215;
    static int lineitemsf10_lines = 59986052;

    static String createTableCSVlineitemSF1 = "CREATE TABLE IF NOT EXISTS csvlineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableCSVlineitemSF10 = "CREATE TABLE IF NOT EXISTS csvlineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableJDBClineitemSF1 = "CREATE TABLE IF NOT EXISTS jdbclineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableJDBClineitemSF10 = "CREATE TABLE IF NOT EXISTS jdbclineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableFDWlineitemSF1 = "CREATE TABLE IF NOT EXISTS fdwlineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableFDWlineitemSF10 = "CREATE TABLE IF NOT EXISTS fdwlineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createForeignTableJDBClineitemSF1 = "CREATE FOREIGN TABLE IF NOT EXISTS jdbcclick1_lineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER jdbcclick1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM click_sf1_lineitem;');";
    static String createForeignTableJDBClineitemSF10 = "CREATE FOREIGN TABLE IF NOT EXISTS jdbcclick1_lineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER jdbcclick1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM click_sf10_lineitem;');";
    static String createCSVExportTablelineitemSF1 = "CREATE TABLE click_export_lineitemsf1(" +
            "l_orderkey Int64, l_partkey Int32, l_suppkey Int32, l_linenumber Int32," +
            "l_returnflag String, l_linestatus String," +
            "l_shipinstruct String," +
            "l_shipmode String, l_comment String) ENGINE=File(CSV);";
    static String createCSVExportTablelineitemSF10 = "CREATE TABLE click_export_lineitemsf10(" +
            "l_orderkey Int64, l_partkey Int32, l_suppkey Int32, l_linenumber Int32," +
            "l_returnflag String, l_linestatus String," +
            "l_shipinstruct String," +
            "l_shipmode String, l_comment String) ENGINE=File(CSV);";
    static String createTableWarplineitemSF1 = "CREATE TABLE IF NOT EXISTS warplineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createTableWarplineitemSF10 = "CREATE TABLE IF NOT EXISTS warplineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44));";
    static String createForeignTableWarplineitemSF1 = "CREATE FOREIGN TABLE IF NOT EXISTS warp1_lineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER warp1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM click_sf1_lineitem;');";
    static String createForeignTableWarplineitemSF10 = "CREATE FOREIGN TABLE IF NOT EXISTS warp1_lineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER warp1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM click_sf10_lineitem;');";



    @BeforeAll
    static void initialize() throws IOException, ClassNotFoundException, SQLException {
        System.out.println("\n\n================= Initialization ===================================\n");
        click1 = new ClickhouseInteractor("click1");
        pg1 = new PostgresInteractor("pg1");

        pg1.initialize(TestPGCH.class.getClassLoader().getResource("joels_dbconfig/pg1.properties").getFile());
        click1.initialize(TestPGCH.class.getClassLoader().getResource("joels_dbconfig/ch1.properties").getFile());

        // create tables for jdbc, csv and fdw lineitem import in PG
        pg1.executeQueryAndPrintResult(createTableCSVlineitemSF1);
        pg1.executeQueryAndPrintResult(createTableCSVlineitemSF10);
        pg1.executeQueryAndPrintResult(createTableFDWlineitemSF1);
        pg1.executeQueryAndPrintResult(createTableFDWlineitemSF10);
        pg1.executeQueryAndPrintResult(createTableJDBClineitemSF1);
        pg1.executeQueryAndPrintResult(createTableJDBClineitemSF10);
        pg1.executeQueryAndPrintResult(createTableWarplineitemSF1);
        pg1.executeQueryAndPrintResult(createTableWarplineitemSF10);

        // create table for csv lineitem export
        click1.executeQueryAndPrintResult("DROP TABLE IF EXISTS click_export_lineitemsf1;");
        click1.executeQueryAndPrintResult(createCSVExportTablelineitemSF1);
        click1.executeQueryAndPrintResult("DROP TABLE IF EXISTS click_export_lineitemsf10;");
        click1.executeQueryAndPrintResult(createCSVExportTablelineitemSF10);

        // create fdw extension on pg1
        pg1.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS clickhouse_fdw;");
        pg1.executeQueryAndPrintResult("CREATE SERVER IF NOT EXISTS fdwclick1 FOREIGN DATA WRAPPER clickhouse_fdw " +
                "OPTIONS(host '10.5.1.31');");
        pg1.executeQueryAndPrintResult("CREATE USER MAPPING FOR postgres SERVER fdwclick1 OPTIONS(" +
                "user 'default', password '');");
        pg1.executeQueryAndPrintResult("IMPORT FOREIGN SCHEMA \"default\" FROM SERVER fdwclick1 INTO public;");

        // create jdbc extension on pg1
        pg1.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS jdbc_fdw;");
        pg1.executeQueryAndPrintResult("CREATE SERVER IF NOT EXISTS jdbcclick1 FOREIGN DATA WRAPPER jdbc_fdw" +
                " OPTIONS(" +
                "drivername 'com.clickhouse.jdbc.ClickHouseDriver'," +
                "url 'jdbc:ch://jclick-1/default?user=default'," +
                "jarfile '/JDBC_FDW/jdbcdriver/clickhouse-jdbc-0.3.2-patch11-all.jar');");
        pg1.executeQueryAndPrintResult("CREATE USER MAPPING FOR postgres SERVER jdbcclick1 OPTIONS(username 'default',password '');");

        // create foreign table for access
        pg1.executeQuery(createForeignTableJDBClineitemSF1);
        pg1.executeQuery(createForeignTableJDBClineitemSF10);

        // Create fdw for Warpdrive on jpg-2...
        pg1.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS warpFDW;");
        pg1.executeQueryAndPrintResult("CREATE SERVER IF NOT EXISTS warp1 " +
                " FOREIGN DATA WRAPPER warpFDW " +
                " OPTIONS (dbname 'db1', host '10.5.1.31', port '13337', fetch_size '250000', use_remote_estimate 'true'," +
                " receiveinrow 'True');");
        pg1.executeQueryAndPrintResult("CREATE USER MAPPING IF NOT EXISTS FOR postgres " +
                "SERVER warp1 OPTIONS(" +
                "user 'default', password '');");
        pg1.executeQuery(createForeignTableWarplineitemSF1);
        pg1.executeQuery(createForeignTableWarplineitemSF10);

    }

    @AfterAll
    static void cleanup() throws ClassNotFoundException {
        System.out.println("\n\n================= Cleanup ===================================\n");
        // Drop tables
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS csvlineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS csvlineitemsf10;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS fdwlineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS fdwlineitemsf10;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS jdbclineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS jdbclineitemsf10;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS warplineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP TABLE IF EXISTS warplineitemsf10;");
        pg1.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcclick1_lineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcclick1_lineitemsf10;");
        pg1.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS warp1_lineitemsf1;");
        pg1.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS warp1_lineitemsf10;");

        // Drop extensions and stuff
        pg1.executeQueryAndPrintResult("DROP SERVER IF EXISTS jdbcclick1 CASCADE;");
        pg1.executeQueryAndPrintResult("DROP EXTENSION IF EXISTS jdbc_fdw CASCADE;");
        pg1.executeQueryAndPrintResult("DROP SERVER IF EXISTS fdwclick1 CASCADE;");
        pg1.executeQueryAndPrintResult("DROP EXTENSION IF EXISTS fdwclick1 CASCADE;");
    }

    //@Test
    public void testBasicQueryClick(){
        System.out.println("\n\n================= testBasicQueryClick ===================================\n");
        try{
            String command = "SELECT * FROM click_sf1_region;";
            click1.executeQueryAndPrintResult(command);
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    //@Test
    public void testBasicWarpQueryClick(){
        System.out.println("\n\n================= testBasicQueryClick ===================================\n");
        try{
            String command = "SELECT * FROM warp1_lineitemsf1;";
            pg1.executeQueryAndPrintResult(command);
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    //@Test
    public void testBasicQueryPGCH(){
        System.out.println("\n\n================= testBasicQueryPGCH ===================================\n");
        try{
            String command = "SELECT * FROM jdbcclick1_lineitemsf1 LIMIT 10;";
            pg1.executeQueryAndPrintResult(command);
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testExportLineitemSF1CSV(){
        System.out.println("\n\n================= testExportLineitemSF1CSV ===================================\n");
        try{
            String command = "INSERT INTO click_export_lineitemsf1 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM click_sf1_lineitem;";
            long start = System.nanoTime();
            click1.executeQueryAndPrintResult(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testImportLineitemSF1CSV(){
        System.out.println("\n\n================= testImportLineitemSF1CSV ===================================\n");
        try{
            String command = "COPY csvlineitemsf1 FROM '/csvdir/click_lineitemsf1.csv' DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pg1.executeQueryAndPrintResult(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testExportLineitemSF10CSV(){
        System.out.println("\n\n================= testExportLineitemSF10CSV ===================================\n");
        try{
            String command = "INSERT INTO click_export_lineitemsf10 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM click_sf10_lineitem;";
            long start = System.nanoTime();
            click1.executeQueryAndPrintResult(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testImportLineitemSF10CSV(){
        System.out.println("\n\n================= testImportLineitemSF10CSV ===================================\n");
        try{
            String command = "COPY csvlineitemsf10 FROM '/csvdir/click_lineitemsf10.csv' DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pg1.executeQueryAndPrintResult(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));
        }catch (Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF1FDW(){
        System.out.println("\n\n================= testReadLineitemSF1FDW ===================================\n");
        try{
            // First read row count of local fdwlineitemsf1
            String readRows = "SELECT count(*) FROM fdwlineitemsf1;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO fdwlineitemsf1 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM click_sf1_lineitem;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));

            // Third, evaluate imported lines.
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(importedLines, lineitemsf1_lines);
        }catch(Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF10FDW(){
        System.out.println("\n\n================= testReadLineitemSF10FDW ===================================\n");
        try{
            // First read row count of local fdwlineitemsf1
            String readRows = "SELECT count(*) FROM fdwlineitemsf10;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO fdwlineitemsf10 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM click_sf10_lineitem;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));

            // Third, evaluate imported lines.
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(importedLines, lineitemsf10_lines);
        }catch(Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF1JDBC(){
        System.out.println("\n\n================= testReadLineitemSF1JDBC ===================================\n");
        try{
            // First read row count of local jdbclineitemsf1
            String readRows = "SELECT count(*) FROM jdbclineitemsf1;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO jdbclineitemsf1 SELECT * FROM jdbcclick1_lineitemsf1;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));

            // Third, evaluate imported lines. 6001215
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(importedLines, lineitemsf1_lines);
        }catch(Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF10JDBC(){
        System.out.println("\n\n================= testReadLineitemSF10JDBC ===================================\n");
        try{
            // First read row count of local jdbclineitemsf1
            String readRows = "SELECT count(*) FROM jdbclineitemsf10;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO jdbclineitemsf10 SELECT * FROM jdbcclick1_lineitemsf10;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: "+ (end - start)/1000000000f +"s") ;
            System.out.println("Raw nanos: "+ (end-start));

            // Third, evaluate imported lines.
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(importedLines, lineitemsf10_lines);
        }catch(Exception e){
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF1WarpDrive() {
        System.out.println("\n\n================= testReadLineitemSF1WarpDrive ===================================\n");
        try {
            // mvn test -Dtest=LocGlobOpt#query test -Dq=3 -Dsf=50 -Dtd=1 -Dmode=hybrid
            // diese properties kann man in der methode auslesen -> so kann eine
            // methode alles testen je nach parametern.

            // First read row count of local warplineitemsf1
            String readRows = "SELECT count(*) FROM warplineitemsf1;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO warplineitemsf1 SELECT * FROM warp1_lineitemsf1;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(lineitemsf1_lines, importedLines);
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF10WarpDrive() {
        System.out.println("\n\n================= testReadLineitemSF10WarpDrive ===================================\n");
        try{
            // mvn test -Dtest=LocGlobOpt#query test -Dq=3 -Dsf=50 -Dtd=1 -Dmode=hybrid
            // diese properties kann man in der methode auslesen -> so kann eine
            // methode alles testen je nach parametern.

            // First read row count of local warplineitemsf1
            String readRows = "SELECT count(*) FROM warplineitemsf10;";
            pg1.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO warplineitemsf10 SELECT * FROM warp1_lineitemsf10;";
            long start = System.nanoTime();
            pg1.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pg1.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(lineitemsf10_lines, importedLines);
        }catch(Exception e){
            e.printStackTrace();
            fail();
        }
    }
}
