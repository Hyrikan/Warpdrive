package postgres;

import core.Interactor;
import core.JDBCProperties;
import core.Utils;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Locale;

public class ODBCSystemHandler implements ForeignSystemHandler {
    public ODBCSystemHandler() {
    }

    @Override
    public void createExtension(JDBCProperties jdbcProperties) {
        String fdw = "CREATE EXTENSION IF NOT EXISTS odbc_fdw";
        try {
            Connection connection = DriverManager.getConnection(jdbcProperties.getUrl(),
                    jdbcProperties.getUser(), jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(fdw);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
        System.out.println(fdw);
    }


    @Override
    public void registerForeignSystem(Interactor interactor, JDBCProperties fjdbcProperties) {
        String sysDrop = "DROP SERVER IF EXISTS " + fjdbcProperties.getSystemName() + " CASCADE";

        //TODO: redesign odbc properties (either through DSN or to odbc-specific config file)
        String sysReg = "CREATE SERVER " + fjdbcProperties.getSystemName() + " " +
                "FOREIGN DATA WRAPPER odbc_fdw " +
                "OPTIONS ( " +
                "odbc_DRIVER '" + fjdbcProperties.getOdbcDriver() + "', " +
                "odbc_SERVER '" + fjdbcProperties.getHostName() + "'," +
                "odbc_PORT '" + fjdbcProperties.getPort() + "'" +
                ");";


        if (fjdbcProperties.getTargetSystem().toLowerCase(Locale.ROOT).equals("hive2")) {
            sysReg = "CREATE SERVER " + fjdbcProperties.getSystemName() + " " +
                    "FOREIGN DATA WRAPPER odbc_fdw " +
                    "OPTIONS ( " +
                    "odbc_DRIVER '" + fjdbcProperties.getOdbcDriver() + "', " +
                    "odbc_HOST '" + fjdbcProperties.getHostName() + "'," +
                    "odbc_PORT '" + fjdbcProperties.getPort() + "'," +
                    "odbc_Schema '" + fjdbcProperties.getDatabaseName() + "'," +
                    "odbc_RowsFetchedPerBlock '250000'," +
                    "odbc_UseNativeQuery '1'," +
                    "odbc_HiveServerType '2'," +
                    "odbc_ServiceDiscoveryMode '0'" +
                    ");";
        }

        /* Alternative using DSN config in odbc.ini

          String sysReg = "CREATE SERVER " + fjdbcProperties.getSystemName() + " " +
            "FOREIGN DATA WRAPPER odbc_fdw " +
            "OPTIONS(dsn 'Hive')";
        */


        String usrReg = "CREATE USER MAPPING FOR " + interactor.getJDBCProperties().getUser() + " " +
                "SERVER " + fjdbcProperties.getSystemName() + " " +
                "OPTIONS (odbc_UID '" + fjdbcProperties.getUser() + "', odbc_PWD '" + fjdbcProperties.getPassword() + "')";

        try {
            Connection connection = DriverManager.getConnection(interactor.getJDBCProperties().getUrl(),
                    interactor.getJDBCProperties().getUser(), interactor.getJDBCProperties().getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(sysDrop);
            stmt.execute(sysReg);
            stmt.execute(usrReg);

            connection.close();
        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }

        System.out.println(sysDrop);
        System.out.println(sysReg);
        System.out.println(usrReg);
    }

    @Override
    public void registerForeignTable(Interactor interactor, JDBCProperties fjdbcProperties, String tableName, String foreignSystemName) {


        String tblDrop = "DROP FOREIGN TABLE IF EXISTS " + tableName + " CASCADE";

/*        String importTable = "IMPORT FOREIGN SCHEMA public " +
                "LIMIT TO (" + tableName + ") " +
                "FROM SERVER " + foreignSystemName + " " +
                "INTO public " +
                "OPTIONS ( " +
                "odbc_DATABASE '" + fjdbcProperties.getDatabaseName() + "', " +
                "table '" + tableName + "', " +
                "sql_query 'select * FROM " + tableName + "', " +
                //"sql_count 'select count(*) from " + tableName + "' " +
                "sql_count 'select 1  '" +
                "); ";*/

        //Alternative using CREATE FOREIGN TABLE:

        String importTable = Utils.getCreateTable(fjdbcProperties, tableName) + " " +
                "SERVER " + foreignSystemName + " " +
                "OPTIONS ( " +
                "odbc_DATABASE '" + fjdbcProperties.getDatabaseName() + "', " +
                "table '" + tableName + "'," +
                "sql_query 'select * FROM " + tableName + "', " +
                "sql_count 'select 1 ' " +
                "); ";


        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            Connection connection = DriverManager.getConnection(interactor.getJDBCProperties().getUrl(),
                    interactor.getJDBCProperties().getUser(), interactor.getJDBCProperties().getPassword());

            Statement stmt = connection.createStatement();

            stmt.execute(tblDrop);
            stmt.execute(importTable);
            //stmt.execute(tblReg);

            connection.close();

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }

        System.out.println(tblDrop);
        System.out.println(importTable);
    }
}
