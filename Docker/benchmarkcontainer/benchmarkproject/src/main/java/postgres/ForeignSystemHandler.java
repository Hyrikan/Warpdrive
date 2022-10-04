package postgres;


import core.Interactor;
import core.JDBCProperties;

public interface ForeignSystemHandler {

    void createExtension(JDBCProperties jdbcProperties);

    void registerForeignSystem(Interactor interactor, JDBCProperties jdbcProperties);

    void registerForeignTable(Interactor interactor, JDBCProperties jdbcProperties, String tableName, String foreignSystemName);
}
