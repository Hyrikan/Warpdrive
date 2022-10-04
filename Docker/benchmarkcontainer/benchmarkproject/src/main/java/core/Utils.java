package core;

import java.sql.*;

public class Utils {

    public static void printResultSet(ResultSet rs) throws SQLException {

        ResultSetMetaData rsmd = rs.getMetaData();

        if (!rs.isBeforeFirst()) {
            System.out.println("Empty ResultSet");
        } else {
            while (rs.next()) {
                StringBuilder sb = new StringBuilder();

                for (int i = 1; i < rsmd.getColumnCount() + 1; i++) {

                    if (i != 1)
                        sb.append("|");
                    sb.append(rs.getString(i));

                }
                System.out.println(sb.toString());
            }
        }

    }

    public static String getCreateTable(JDBCProperties jdbcProperties, String tableName) {


        try {
            Connection connection = DriverManager.getConnection(jdbcProperties.getUrl(),
                    jdbcProperties.getUser(), jdbcProperties.getPassword());

            Statement stmt = connection.createStatement();
            ResultSet rs = stmt.executeQuery("SELECT * FROM " + tableName + " LIMIT 1");

            ResultSetMetaData rsmd = rs.getMetaData();

            rs.next();
            String createStr = "CREATE FOREIGN TABLE " + tableName + "(";
            for (int i = 1; i < rsmd.getColumnCount() + 1; i++) {
                String typeName = JDBCType.valueOf(rsmd.getColumnType(i)).getName();

                //dirty fixes
                //if (typeName.equals("DECIMAL"))
                //    typeName = "DOUBLE";
                if (typeName.equals("NUMERIC"))
                    typeName = "DECIMAL";

                //typeName = "DECIMAL(" + rsmd.getPrecision(i) + "," + rsmd.getScale(i) + ")";
                createStr += rsmd.getColumnName(i) + " " + typeName;

                if (typeName.equals("CHAR") || typeName.equals("VARCHAR")) {
                    int size = 255;
                    if (rsmd.getColumnDisplaySize(i) < 255)
                        size = rsmd.getColumnDisplaySize(i);

                    createStr += "(" + size + ")";
                }

                createStr += ",";

                /*System.out.println("TYPE: " + rsmd.getColumnType(i));
                System.out.println("JDBC TYPE: " + typeName);
                System.out.println("TYPENAME: " + rsmd.getColumnTypeName(i));
                System.out.println("COL NAME: " + rsmd.getColumnName(i));
                System.out.println("SCHEMA NAME: " + rsmd.getSchemaName(i));
                System.out.println("-------------");*/
            }
            createStr = createStr.replaceAll(",$", "");
            createStr += ")";

            return createStr;

        } catch (SQLException throwables) {
            throwables.printStackTrace();
        }

        return "";
    }


}
