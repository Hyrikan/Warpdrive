package core;

import java.util.HashMap;

public class SystemCatalog {

    HashMap<String, Interactor> catalog;

    public SystemCatalog() {
        this.catalog = new HashMap<>();
    }

    public void add(Interactor interactor) {
        this.catalog.put(interactor.getSystemName(), interactor);
    }

    public HashMap<String, Interactor> getAll() {
        return this.catalog;
    }

    public Interactor get(String systemName) {
        return this.catalog.get(systemName);
    }
}
