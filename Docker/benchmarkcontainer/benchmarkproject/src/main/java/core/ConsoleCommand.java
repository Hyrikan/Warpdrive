package core;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;


public class ConsoleCommand {

    public static void runCommand(String command) {
        Process proc;
        try {
            proc = Runtime.getRuntime().exec(command);
            BufferedReader reader =
                    new BufferedReader(new InputStreamReader(proc.getInputStream()));

            String line = "";
            while((line = reader.readLine()) != null) {
                System.out.print(line + "\n");
            }
            proc.waitFor();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
