import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.postgresql.jdbc.PgResultSet;
import postgres.PostgresInteractor;
import sun.font.TrueTypeFont;

import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;

import static org.junit.jupiter.api.Assertions.*;

public class TestPGPG {
    static PostgresInteractor pgint;
    static PostgresInteractor pgrec;
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
    static String createForeignTableJDBClineitemSF1 = "CREATE FOREIGN TABLE IF NOT EXISTS jdbcpos1_lineitemsf1(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER jdbcpos1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf1_lineitem;');";
    static String createForeignTableJDBClineitemSF10 = "CREATE FOREIGN TABLE IF NOT EXISTS jdbcpos1_lineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER jdbcpos1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf10_lineitem;');";
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
            "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf1_lineitem;');";
    static String createForeignTableWarplineitemSF10 = "CREATE FOREIGN TABLE IF NOT EXISTS warp1_lineitemsf10(" +
            "l_orderkey BIGINT, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER," +
            "l_returnflag CHARACTER(1), l_linestatus CHARACTER(1)," +
            "l_shipinstruct CHARACTER(25)," +
            "l_shipmode CHARACTER(10), l_comment VARCHAR(44)) SERVER warp1 OPTIONS(" +
            "query 'SELECT  l_orderkey, l_partkey, l_suppkey, " +
            "l_linenumber, l_returnflag, l_linestatus, " +
            "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf10_lineitem;');";

    @BeforeAll
    static void initialize() throws IOException, ClassNotFoundException, SQLException {
        System.out.println("\n\n================= Initialization ===================================\n");
        pgint = new PostgresInteractor("PG1");
        pgrec = new PostgresInteractor("PG2");
        pgint.initialize(TestPGPG.class.getClassLoader().getResource("joels_dbconfig/pg1.properties").getFile());
        pgrec.initialize(TestPGPG.class.getClassLoader().getResource("joels_dbconfig/pg2.properties").getFile());

        //create tables on jpg-2 to import sf1 and sf10 data into local tables.
        pgrec.executeQueryAndPrintResult(createTableCSVlineitemSF1);
        pgrec.executeQueryAndPrintResult(createTableCSVlineitemSF10);
        pgrec.executeQueryAndPrintResult(createTableJDBClineitemSF1);
        pgrec.executeQueryAndPrintResult(createTableJDBClineitemSF10);
        pgrec.executeQueryAndPrintResult(createTableFDWlineitemSF1);
        pgrec.executeQueryAndPrintResult(createTableFDWlineitemSF10);
        pgrec.executeQueryAndPrintResult(createTableWarplineitemSF1);
        pgrec.executeQueryAndPrintResult(createTableWarplineitemSF10);

        // Create FDW and foreign table for lineitemSF1 and SF10 on jpg-2 to read from jpg-1
        pgrec.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS postgres_fdw;");
        pgrec.executeQueryAndPrintResult("CREATE SERVER IF NOT EXISTS pos1\n" +
                " FOREIGN DATA WRAPPER postgres_fdw\n" +
                " OPTIONS (dbname 'db1', host 'jpg-1', port '5432', fetch_size '250000', use_remote_estimate 'true');");
        pgrec.executeQueryAndPrintResult("CREATE USER MAPPING FOR postgres\n" +
                "  SERVER pos1\n" +
                "  OPTIONS (user 'postgres', password 'post1234');");
        pgrec.executeQueryAndPrintResult("IMPORT FOREIGN SCHEMA public LIMIT TO (pg1_sf1_lineitem) FROM SERVER pos1 INTO public;");
        pgrec.executeQueryAndPrintResult("IMPORT FOREIGN SCHEMA public LIMIT TO (pg1_sf10_lineitem) FROM SERVER pos1 INTO public;");

        // Create jdbc fdw and tables for lineitemSF1 and SF10 on jpg-2 to read from jpg-1
        pgrec.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS jdbc_fdw;");
        pgrec.executeQueryAndPrintResult("CREATE SERVER jdbcpos1 FOREIGN DATA WRAPPER jdbc_fdw OPTIONS(" +
                "drivername 'org.postgresql.Driver'," +
                "url 'jdbc:postgresql://10.5.1.21:5432/db1'," +
                "querytimeout '0'," +
                "jarfile '/JDBC_FDW/jdbcdriver/postgresql-42.3.3.jar');");
        pgrec.executeQueryAndPrintResult("CREATE USER MAPPING FOR postgres " +
                "SERVER jdbcpos1 OPTIONS(" +
                "username 'postgres', password 'post1234');");
        pgrec.executeQuery(createForeignTableJDBClineitemSF1);
        pgrec.executeQuery(createForeignTableJDBClineitemSF10);

        // Create fdw for Warpdrive on jpg-2...
        pgrec.executeQueryAndPrintResult("CREATE EXTENSION IF NOT EXISTS warpFDW;");
        pgrec.executeQueryAndPrintResult("CREATE SERVER IF NOT EXISTS warp1 " +
                " FOREIGN DATA WRAPPER warpFDW " +
                " OPTIONS (dbname 'db1', host '10.5.1.21', port '13337', fetch_size '250000', use_remote_estimate 'true'," +
                " receiveinrow 'True');");
        pgrec.executeQueryAndPrintResult("CREATE USER MAPPING IF NOT EXISTS FOR postgres " +
                "SERVER warp1 OPTIONS(" +
                "user 'postgres', password 'post1234');");
        pgrec.executeQuery(createForeignTableWarplineitemSF1);
        pgrec.executeQuery(createForeignTableWarplineitemSF10);


    }

    @AfterAll
    static void cleanTables() throws ClassNotFoundException {
        System.out.println("\n\n================= Cleanup ===================================\n");
        // Drop tables on jpg-2
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS csvlineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS csvlineitemsf10;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS jdbclineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS jdbclineitemsf10;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS fdwlineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS fdwlineitemsf10;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS warplineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP TABLE IF EXISTS warplineitemsf10;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcpos1_lineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcpos1_lineitemsf10;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS warp1_lineitemsf1;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS warp1_lineitemsf10;");

        // Drop native FDW on jpg-2
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS pg1_sf1_lineitem CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS pg1_sf10_lineitem CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP SERVER IF EXISTS pos1 CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP EXTENSION IF EXISTS postgres_fdw CASCADE;");

        // Drop jdbc FDW on jpg-2
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcpos1_sf1_region CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP FOREIGN TABLE IF EXISTS jdbcpos1_sf10_region CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP SERVER IF EXISTS jdbcpos1 CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP EXTENSION IF EXISTS jdbc_fdw CASCADE;");

        // Drop warpFDW on jpg-2
        pgrec.executeQueryAndPrintResult("DROP SERVER IF EXISTS warp1 CASCADE;");
        pgrec.executeQueryAndPrintResult("DROP EXTENSION IF EXISTS warpFDW CASCADE;");
    }

    //@Test
    public void testSQLCommand() {
        System.out.println("\n\n================= testSQLCommand ===================================\n");
        try {
            String command = "SELECT * FROM pg1_sf1_region;";
            pgint.executeQueryAndPrintResult(command);
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testExportLineitemSF1CSV() {
        System.out.println("\n\n================= testExportLineitemSF1CSV ===================================\n");
        try {
            String exportcommand = "COPY (SELECT  l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf1_lineitem) TO '/csvdir/lineitemsf1.csv'  WITH DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pgint.executeQuery(exportcommand);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testImportLineitemSF1CSV() {
        System.out.println("\n\n================= testImportLineitemSF1CSV ===================================\n");
        try {
            String importcommand = "COPY csvlineitemsf1 FROM '/csvdir/lineitemsf1.csv' DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pgrec.executeQueryAndPrintResult(importcommand);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testExportLineitemSF10CSV() {
        System.out.println("\n\n================= testExportLineitemSF10CSV ===================================\n");
        try {
            String exportcommand = "COPY (SELECT  l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf10_lineitem) TO '/csvdir/lineitemsf10.csv'  WITH DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pgint.executeQuery(exportcommand);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testImportLineitemSF10CSV() {
        System.out.println("\n\n================= testImportLineitemSF10CSV ===================================\n");
        try {
            String importcommand = "COPY csvlineitemsf10 FROM '/csvdir/lineitemsf10.csv' DELIMITER ',' CSV HEADER;";
            long start = System.nanoTime();
            pgrec.executeQueryAndPrintResult(importcommand);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF1JDBC() {
        System.out.println("\n\n================= testReadLineitemSF1JDBC ===================================\n");
        try {
            // First read row count of local jdbclineitemsf1
            String readRows = "SELECT count(*) FROM jdbclineitemsf1;";
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO jdbclineitemsf1 SELECT * FROM jdbcpos1_lineitemsf1;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(lineitemsf1_lines, importedLines);
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    //@Test // does not finish...
    public void testReadLineitemSF10JDBC() {
        System.out.println("\n\n================= testReadLineitemSF10JDBC ===================================\n");
        try {
            // First read row count of local jdbclineitemsf1
            String readRows = "SELECT count(*) FROM jdbclineitemsf10;";
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO jdbclineitemsf10 SELECT * FROM jdbcpos1_lineitemsf10;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(lineitemsf10_lines, importedLines);
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testReadLineitemSF1FDW() {
        System.out.println("\n\n================= testReadLineitemSF1FDW ===================================\n");
        try {
            // mvn test -Dtest=LocGlobOpt#query test -Dq=3 -Dsf=50 -Dtd=1 -Dmode=hybrid
            // diese properties kann man in der methode auslesen -> so kann eine
            // methode alles testen je nach parametern.

            // First read row count of local fdwlineitemsf1
            String readRows = "SELECT count(*) FROM fdwlineitemsf1;";
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO fdwlineitemsf1 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf1_lineitem;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
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
    public void testReadLineitemSF10FDW() {
        System.out.println("\n\n================= testReadLineitemSF10FDW ===================================\n");
        try {
            // mvn test -Dtest=LocGlobOpt#query test -Dq=3 -Dsf=50 -Dtd=1 -Dmode=hybrid
            // diese properties kann man in der methode auslesen -> so kann eine
            // methode alles testen je nach parametern.

            // First read row count of local fdwlineitemsf1
            String readRows = "SELECT count(*) FROM fdwlineitemsf10;";
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO fdwlineitemsf10 SELECT l_orderkey, l_partkey, l_suppkey, " +
                    "l_linenumber, l_returnflag, l_linestatus, " +
                    "l_shipinstruct, l_shipmode, l_comment FROM pg1_sf10_lineitem;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
            rs.next();
            int importedLines = rs.getInt(1);
            System.out.println(importedLines);
            assertEquals(lineitemsf10_lines, importedLines);
        } catch (Exception e) {
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
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO warplineitemsf1 SELECT * FROM warp1_lineitemsf1;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
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
            pgrec.executeQueryAndPrintResult(readRows);

            // Second, execute import test and measure time.
            String command = "INSERT INTO warplineitemsf10 SELECT * FROM warp1_lineitemsf10;";
            long start = System.nanoTime();
            pgrec.executeQuery(command);
            long end = System.nanoTime();
            System.out.println("Time elapsed: " + (end - start) / 1000000000f + "s");
            System.out.println("Raw nanos: " + (end - start));

            // Third, evaluate imported lines.
            ResultSet rs = pgrec.executeQueryAndReturnResultSet(readRows);
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
