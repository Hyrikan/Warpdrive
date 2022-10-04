package postgres;


import core.Interactor;
import core.JDBCProperties;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class PostgresSystemHandler implements ForeignSystemHandler {
    @Override
    public void createExtension(JDBCProperties jdbcProperties) {
        String fdw = "CREATE EXTENSION IF NOT EXISTS postgres_fdw";
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

        String sysReg = "CREATE SERVER " + fjdbcProperties.getSystemName() + " FOREIGN DATA WRAPPER postgres_fdw " +
                "OPTIONS( " +
                "host '" + fjdbcProperties.getHostName() + "'," +
                "port '" + fjdbcProperties.getPort() + "'," +
                "dbname '" + fjdbcProperties.getDatabaseName() + "'," +
                "fetch_size '250000'," +
                "use_remote_estimate 'true'" +
                ")";


        String usrReg = "CREATE USER MAPPING for " + interactor.getJDBCProperties().getUser() + " " +
                "SERVER " + fjdbcProperties.getSystemName() + " " +
                "OPTIONS(user '" + fjdbcProperties.getUser() + "', " +
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

        String tblReg = "IMPORT FOREIGN SCHEMA public " +
                "LIMIT TO (" + tableName + ")" + " " +
                "FROM SERVER " + foreignSystemName + " INTO public";

        try {


            Connection connection = DriverManager.getConnection(interactor.getJDBCProperties().getUrl(),
                    interactor.getJDBCProperties().getUser(), interactor.getJDBCProperties().getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(tblDrop);
            stmt.execute(tblReg);

            connection.close();

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
    }
}
