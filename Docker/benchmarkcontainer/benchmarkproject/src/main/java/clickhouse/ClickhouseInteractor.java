package clickhouse;

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

public class ClickhouseInteractor implements Interactor {

    public JDBCProperties jdbcProperties;
    public String systemName;
    public ArrayList<String> registeredSystems;
    public Properties prop;

    public ClickhouseInteractor(String systemName) {
        this.systemName = systemName;
        this.jdbcProperties = new JDBCProperties();
        this.registeredSystems = new ArrayList<>();
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
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void registerLocalView(String viewName, String query) {

    }

    @Override
    public void registerJoinView(String viewName, String tableA, String tableB, String joinOn) {

    }

    @Override
    public void registerForeignTable(SystemCatalog sc, String systemName, String tableName) {

    }

    @Override
    public void executeQueryAndPrintResult(String query) throws ClassNotFoundException {
        try {
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

    @Override
    public JDBCProperties getJDBCProperties() {
        return this.jdbcProperties;
    }

    @Override
    public String getSystemName() {
        return systemName;
    }
}
