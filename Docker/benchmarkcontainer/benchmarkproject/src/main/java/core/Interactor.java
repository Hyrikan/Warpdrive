package core;

public interface Interactor {

    void registerLocalView(String viewName, String query);

    void registerJoinView(String viewName, String tableA, String tableB, String joinOn);

    void registerForeignTable(SystemCatalog sc, String systemName, String tableName);

    void executeQueryAndPrintResult(String query) throws ClassNotFoundException;

    JDBCProperties getJDBCProperties();

    String getSystemName();
}
