/*
 * Copyright 2016 Thomas Krause.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.korpling.annis.benchmark.generator;

import annis.ql.parser.AnnisParserAntlr;
import annis.ql.parser.QueryData;
import annis.ql.parser.SemanticValidator;
import com.google.common.io.Files;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import java.util.ResourceBundle;
import java.util.Set;
import javafx.beans.property.ReadOnlyObjectWrapper;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ListChangeListener.Change;
import javafx.collections.ObservableList;
import javafx.collections.transformation.FilteredList;
import javafx.collections.transformation.SortedList;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Parent;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;
import javafx.scene.control.CheckBox;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.scene.input.Clipboard;
import javafx.scene.input.ClipboardContent;
import javafx.stage.DirectoryChooser;
import javafx.stage.FileChooser;
import javafx.util.Callback;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * FXML Controller class
 *
 * @author thomas
 */
public class QuerySetViewController implements Initializable
{

  private final Logger log = LoggerFactory.getLogger(
    QuerySetViewController.class);

  @FXML
  private Parent root;

  private final FileChooser fileChooser = new FileChooser();
  private final DirectoryChooser dirChooser = new DirectoryChooser();
  
  private final FileChooser.ExtensionFilter logFilter = new FileChooser.ExtensionFilter(
    "Query log (*.log)", "*.log");
  

  @FXML
  private TableView<Query> tableView;

  @FXML
  private TableColumn<Query, String> nameColumn;

  @FXML
  private TableColumn<Query, String> aqlColumn;

  @FXML
  private TableColumn<Query, Set<String>> corpusColumn;

  @FXML
  private TableColumn<Query, Optional<Long>> execTimeColumn;
  
  @FXML
  private TableColumn<Query, Boolean> validColumn;

  @FXML
  private TableColumn<Query, Optional<Long>> nrResultsColumn;

  @FXML
  private TextField corpusFilter;

  @FXML
  private CheckBox oneCorpusFilter;
  
  @FXML
  private CheckBox onlyValidFilter;

  @FXML
  private Label counterLabel;
  

  private final ObservableList<Query> queries = FXCollections.
    observableArrayList();
  
  private AnnisParserAntlr parser;

  /**
   * Initializes the controller class.
   */
  @Override
  public void initialize(URL url, ResourceBundle rb)
  {
    parser = new AnnisParserAntlr();
    parser.setPrecedenceBound(50);
    parser.setPostProcessors(Arrays.asList(new SemanticValidator()));  

    nameColumn.setCellValueFactory(new PropertyValueFactory<>("name"));
    aqlColumn.setCellValueFactory(new PropertyValueFactory<>("aql"));
    corpusColumn.setCellValueFactory(new PropertyValueFactory<>("corpora"));
    execTimeColumn.setCellValueFactory(new PropertyValueFactory<>("executionTime"));
    nrResultsColumn.setCellValueFactory(new PropertyValueFactory<>("count"));    
    
    nameColumn.setCellFactory(TextFieldTableCell.forTableColumn());
    aqlColumn.setCellFactory(TextAreaTableCell.forTableColumn());
    corpusColumn.setCellFactory(TextFieldTableCell.forTableColumn(new StringSetConverter()));
    execTimeColumn.setCellFactory(TextFieldTableCell.forTableColumn(new OptionalLongConverter()));
    nrResultsColumn.setCellFactory(TextFieldTableCell.forTableColumn(new OptionalLongConverter()));
    validColumn.setCellValueFactory(param -> new ReadOnlyObjectWrapper<>(param.getValue().getJson() != null));

    
    
    nameColumn.setOnEditCommit((TableColumn.CellEditEvent<Query, String> event) ->
    {
      event.getRowValue().setName(event.getNewValue());
    });
    aqlColumn.setOnEditCommit((TableColumn.CellEditEvent<Query, String> event) ->
    {
      event.getRowValue().setAql(event.getNewValue());
    });
    corpusColumn.setOnEditCommit((TableColumn.CellEditEvent<Query, Set<String>> event) ->
    {
      event.getRowValue().setCorpora(event.getNewValue());
    });
    execTimeColumn.setOnEditCommit((TableColumn.CellEditEvent<Query, Optional<Long>> event) ->
    {
      event.getRowValue().setExecutionTime(event.getNewValue());
    });
    nrResultsColumn.setOnEditCommit((TableColumn.CellEditEvent<Query, Optional<Long>> event) ->
    {
      event.getRowValue().setCount(event.getNewValue());
    });
    

    FilteredList<Query> filteredQueries = new FilteredList<>(queries, p -> true);
    SortedList<Query> sortedQueries = new SortedList<>(filteredQueries);

    sortedQueries.comparatorProperty().bind(tableView.comparatorProperty());

    corpusFilter.textProperty().addListener(observable
      -> 
      {
        setFilterPredicate(filteredQueries);
    });
    oneCorpusFilter.selectedProperty().addListener(observable
      -> 
      {
        setFilterPredicate(filteredQueries);
    });
    onlyValidFilter.selectedProperty().addListener(observable
      -> 
      {
        setFilterPredicate(filteredQueries);
    });
    
    tableView.setItems(sortedQueries);

    filteredQueries.addListener((Change<? extends Query> change)
      -> 
      {
        counterLabel.textProperty().set("" + filteredQueries.size());
    });
  }

  private void setFilterPredicate(FilteredList<Query> filteredQueries)
  {
    if (filteredQueries != null)
    {
      filteredQueries.setPredicate(query
        -> 
        {

          String corpusFilterText = corpusFilter.textProperty().get();
          boolean allowSingleCorpusOnly = oneCorpusFilter.selectedProperty().
            get();
          boolean allowWithJsonOnly = onlyValidFilter.selectedProperty().get();

          if (allowSingleCorpusOnly && query.getCorpora().size() > 1)
          {
            return false;
          }
          
          if(allowWithJsonOnly && query.getJson() == null)
          {
            return false;
          }

          if (corpusFilterText != null && !corpusFilterText.isEmpty())
          {
            if (!query.getCorpora().contains(corpusFilterText))
            {
              return false;
            }
          }

          return true;

      });
    }
  }

  @FXML
  public void filterByCorpusOfQuery(ActionEvent evt)
  {
    Query q = tableView.getSelectionModel().getSelectedItem();
    if (q != null && !q.getCorpora().isEmpty())
    {
      evt.consume();
      oneCorpusFilter.selectedProperty().set(true);
      corpusFilter.textProperty().set(q.getCorpora().iterator().next());
    }
  }
  
  @FXML
  public void copySelectedQuery(ActionEvent evt)
  {
    Query q = tableView.getSelectionModel().getSelectedItem();
    if (q != null && q.getAql() != null)
    {
      evt.consume();
      ClipboardContent content = new ClipboardContent();
      content.putString(q.getAql());
      Clipboard.getSystemClipboard().setContent(content);
    }
  }
  
  @FXML
  public void addNewQuery(ActionEvent evt)
  {
    queries.add(new Query());
  }
  
  @FXML
  public void deleteSelectedQuery(ActionEvent evt)
  {
    List<Query> q = tableView.getSelectionModel().getSelectedItems();
    if (q != null && !q.isEmpty())
    {
      evt.consume();
      queries.removeAll(q);
    }
  }

  @FXML
  public void loadQueryLog(ActionEvent evt)
  {
    fileChooser.setTitle("Open Query Log");
    fileChooser.getExtensionFilters().clear();
    fileChooser.getExtensionFilters().add(logFilter);
    fileChooser.setSelectedExtensionFilter(logFilter);

    File selectedFile = fileChooser.showOpenDialog(root.getScene().getWindow());
    if (selectedFile != null)
    {
      try
      {
        List<Query> parsedQueries = Files.readLines(selectedFile,
          StandardCharsets.UTF_8,
          new QueryLogParser());

        queries.clear();
        queries.addAll(parsedQueries);

      }
      catch (IOException ex)
      {
        log.error(null, ex);
        new Alert(Alert.AlertType.ERROR, "Could not parse file: " + ex.
          getMessage(), ButtonType.OK).showAndWait();

      }
    }
  }
  
  @FXML
  public void load(ActionEvent evt)
  {
    dirChooser.setTitle("Set directory");
    
    File dir = dirChooser.showDialog(root.getScene().getWindow());
    if(dir != null)
    {
      List<Query> loaded = QuerySetPersistance.loadQuerySet(dir);
      queries.clear();
      queries.addAll(loaded);
    }
  }
  
  @FXML
  public void export(ActionEvent evt)
  {
    dirChooser.setTitle("Set export directory");
    
    File dir = dirChooser.showDialog(root.getScene().getWindow());
    if(dir != null)
    {
      int successCounter = QuerySetPersistance.writeQuerySet(dir, queries);
      int errorCounter =  queries.size() - successCounter;

      if(errorCounter == 0)
      {
        new Alert(AlertType.INFORMATION, "All queries exported successfully", ButtonType.OK).showAndWait();
      }
      else
      {
        new Alert(AlertType.ERROR, "" + errorCounter + " had errors", ButtonType.OK).showAndWait();
      }
    }
  }
  
  @FXML
  public void parseJSON(ActionEvent evt)
  {
    // only parse the visible items
    queries.stream().
      forEach((q) ->
    {
      try
      {
        q.setJson(null);
        QueryData queryData = parser.parse(q.getAql(), null);
        String asJSON = QueryToJSON.serializeQuery(queryData);
        q.setJson(asJSON);
      }
      catch(Exception ex)
      {
        log.error("Could not create json", ex);
      }
    });
    validColumn.setVisible(false);
    validColumn.setVisible(true);
  }

}