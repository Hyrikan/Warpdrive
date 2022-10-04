import core.ConsoleCommand;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class TestConsoleCommand {

    @Test
    public void testRunCommandInConsole(){
        ConsoleCommand.runCommand("echo 'Hallo, Welt!'");
    }
}
