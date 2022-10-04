package postgres;

import core.Interactor;
import core.JDBCProperties;
import core.SystemCatalog;
import core.Utils;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.sql.*;
import java.util.ArrayList;
import java.util.Properties;

public class PostgresInteractor implements Interactor {

    public JDBCProperties jdbcProperties;
    public String systemName;
    public ArrayList<String> registeredSystems;
    public Properties prop;

    public PostgresInteractor(String systemName) {

        this.systemName = systemName;
        this.jdbcProperties = new JDBCProperties();
        this.registeredSystems = new ArrayList<>();
    }

    @Override
    public String getSystemName() {
        return systemName;
    }

    public void initialize(String propertiesFile) throws IOException {

        try (InputStream input = new FileInputStream(propertiesFile)) {

            prop = new Properties();
            prop.load(input);

            this.jdbcProperties.setSystemName(this.systemName);
            this.jdbcProperties.setPassword(prop.getProperty("password", ""));
            this.jdbcProperties.setUser(prop.getProperty("user", ""));
            this.jdbcProperties.setUrl(prop.getProperty("url", ""));
            this.jdbcProperties.setDriverName(prop.getProperty("driverName", ""));
            this.jdbcProperties.setDriverJar(prop.getProperty("driverJar", ""));
            this.jdbcProperties.setOdbcDriver(prop.getProperty("odbcDriver", ""));


            // hopefully removes user canceled queries due to some timeout
            DriverManager.setLoginTimeout(0);
            //instantiateWrapper();

        } catch (IOException ex) {
            ex.printStackTrace();
        }


    }

    /*public void instantiateWrapper() {
        String fdw = "CREATE EXTENSION IF NOT EXISTS jdbc_fdw";
        try {
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.jdbcProperties.getUser(), this.jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(fdw);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
    }*/


    @Override
    public void registerLocalView(String viewName, String query) {
        String dropView = "DROP VIEW IF EXISTS " + viewName + " CASCADE";
        String localView = "CREATE VIEW " + viewName + " AS " + query;
        try {
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.jdbcProperties.getUser(), this.jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();

            stmt.execute(dropView);
            stmt.execute(localView);

            connection.close();

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }

    }

    @Override
    public void registerJoinView(String viewName, String tableA, String tableB, String joinOn) {

        String dropView = "DROP VIEW IF EXISTS " + viewName + " CASCADE";
        String joinView = "CREATE VIEW " + viewName + " AS SELECT * FROM " + tableA + "," + tableB + " WHERE " + joinOn;

        try {
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.jdbcProperties.getUser(), this.jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();
            stmt.execute(dropView);
            stmt.execute(joinView);

            connection.close();

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }


    }

    @Override
    public void registerForeignTable(SystemCatalog sc, String foreignSystemName, String tableName) {

        ForeignSystemHandler fsh;
        if (sc.get(foreignSystemName).getJDBCProperties().getUrl().contains("postgres"))
            fsh = new PostgresSystemHandler();
        else
            fsh = new ODBCSystemHandler();

        fsh.createExtension(this.jdbcProperties);

        if (!this.registeredSystems.contains(foreignSystemName)) {
            fsh.registerForeignSystem(this, sc.get(foreignSystemName).getJDBCProperties());
            this.registeredSystems.add(foreignSystemName);
        }

        fsh.registerForeignTable(this, sc.get(foreignSystemName).getJDBCProperties(), tableName, foreignSystemName);

    }


    @Override
    public JDBCProperties getJDBCProperties() {
        return this.jdbcProperties;
    }

    @Override
    public void executeQueryAndPrintResult(String query) throws ClassNotFoundException {

        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            System.out.println(this.systemName + " Executing query: ");
            System.out.println(query);
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.prop);

            connection.setAutoCommit(false);
            Statement stmt = connection.createStatement();
            if (query.contains("SELECT")) {
                ResultSet rs = stmt.executeQuery(query);
                Utils.printResultSet(rs);
            } else
                stmt.execute(query);
            connection.commit();
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
    }

    public ResultSet executeQueryAndReturnResultSet(String query) throws ClassNotFoundException {

        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            System.out.println(this.systemName + " Executing query: ");
            System.out.println(query);
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.prop);

            Statement stmt = connection.createStatement();
            if (query.contains("SELECT")) {
                ResultSet rs = stmt.executeQuery(query);
                //Utils.printResultSet(rs);
                return rs;
            } else
                stmt.execute(query);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
        return null;

    }
    public ResultSet executeQuery(String query) throws ClassNotFoundException {

        try {
            //Class.forName(this.jdbcProperties.getDriverName());

            System.out.println(this.systemName + " Executing query: ");
            System.out.println(query);
            Connection connection = DriverManager.getConnection(this.jdbcProperties.getUrl(),
                    this.prop);

            Statement stmt = connection.createStatement();
            stmt.execute(query);
            connection.close();


        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }
        return null;

    }
}
