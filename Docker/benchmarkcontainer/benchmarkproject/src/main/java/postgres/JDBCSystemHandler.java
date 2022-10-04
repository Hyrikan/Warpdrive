package postgres;

import core.Interactor;
import core.JDBCProperties;
import core.Utils;
import postgres.ForeignSystemHandler;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class JDBCSystemHandler implements ForeignSystemHandler {
    public JDBCSystemHandler() {
    }

    @Override
    public void createExtension(JDBCProperties jdbcProperties) {
        String fdw = "CREATE EXTENSION IF NOT EXISTS jdbc_fdw";
        try {
            Connection connection = DriverManager.getConnection(jdbcProperties.getUrl(),
                    jdbcProperties.getUser(), jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(fdw);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
    }

    @Override
    public void registerForeignSystem(Interactor interactor, JDBCProperties fjdbcProperties) {

        String sysDrop = "DROP SERVER IF EXISTS " + fjdbcProperties.getSystemName() + " CASCADE";

        String sysReg = "CREATE SERVER " + fjdbcProperties.getSystemName() + " FOREIGN DATA WRAPPER jdbc_fdw " +
                "OPTIONS( " +
                "drivername '" + fjdbcProperties.getDriverName() + "'," +
                "url '" + fjdbcProperties.getUrl() + "'," +
                "querytimeout '360'," +
                "jarfile '" + fjdbcProperties.getDriverJar() + "'," +
                "maxheapsize '3000'" +
                ")";

        String usrReg = "CREATE USER MAPPING for " + interactor.getJDBCProperties().getUser() + " " +
                "SERVER " + fjdbcProperties.getSystemName() + " " +
                "OPTIONS(username '" + fjdbcProperties.getUser() + "', " +
                "password '" + fjdbcProperties.getPassword() + "')";

        //String schemaReg = "CREATE SCHEMA IF NOT EXISTS " + foreignSystemName + "_schema ";

        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            Connection connection = DriverManager.getConnection(interactor.getJDBCProperties().getUrl(),
                    interactor.getJDBCProperties().getUser(), interactor.getJDBCProperties().getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(sysDrop);
            stmt.execute(sysReg);
            stmt.execute(usrReg);
            //stmt.execute(schemaReg);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }

    }

    @Override
    public void registerForeignTable(Interactor interactor, JDBCProperties fjdbcProperties, String tableName, String foreignSystemName) {
        String tblDrop = "DROP FOREIGN TABLE IF EXISTS " + tableName + " CASCADE";

        /*String tblReg = "IMPORT FOREIGN SCHEMA public " +
                "LIMIT TO (" + tableName + ")" + " " +
                "FROM SERVER " + foreignSystemName + " INTO public";*/

        String createTable = Utils.getCreateTable(fjdbcProperties, tableName) + " " +
                "SERVER " + foreignSystemName + " " +
                "OPTIONS (query 'SELECT * FROM " + tableName + "')";
        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            Connection connection = DriverManager.getConnection(interactor.getJDBCProperties().getUrl(),
                    interactor.getJDBCProperties().getUser(), interactor.getJDBCProperties().getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(tblDrop);
            stmt.execute(createTable);
            //stmt.execute(tblReg);

            connection.close();

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
    }
}
