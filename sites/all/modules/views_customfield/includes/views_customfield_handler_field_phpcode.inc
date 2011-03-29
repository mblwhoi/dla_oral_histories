<?php
// $Id: views_customfield_handler_field_phpcode.inc,v 1.1.2.7 2009/07/16 16:22:21 kratib Exp $

/**
 * @file
 * Contains the 'customfield' phpcode field handler.
 */

/**
 * Field handler to display row-specific custom text using PHP.
 */
define('VIEWS_CUSTOMFIELD_SORTABLE_NO', 0);
define('VIEWS_CUSTOMFIELD_SORTABLE_NUMERIC', 1);
define('VIEWS_CUSTOMFIELD_SORTABLE_ALPHA', 2);
define('VIEWS_CUSTOMFIELD_SORTABLE_ALPHA_NOCASE', 3);
define('VIEWS_CUSTOMFIELD_SORTABLE_NAT', 4);
define('VIEWS_CUSTOMFIELD_SORTABLE_NAT_NOCASE', 5);

class views_customfield_handler_field_phpcode extends views_handler_field {
  var $static;
  
  function query() {
    $this->field_alias = 'customfield_phpcode_'. $this->position;
  }
  
  function option_definition() {
    $options = parent::option_definition();
    $options['value'] = array('default' => '');
    $options['sortable'] = array('default' => VIEWS_CUSTOMFIELD_SORTABLE_NO);
    return $options;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['value'] = array(
      '#type' => 'textarea',
      '#title' => t('Value'),
      '#default_value' => $this->options['value'],
      '#rows' => 5,
      '#description' => 
        t('The text that should be displayed.')
        .' '.t('Include &lt;?php ?&gt; delimiters when using PHP code.')
        .' '.t('Available variables:').'<br/>'
        .t('$data: contains the retrieved record from the database (e.g. $data->nid).').'<br/>'
        .t('$static: can be used to store reusable data per row.')
      ,
    );
    $form['sortable'] = array(
      '#type' => 'radios',
      '#title' => t('Sortable'),
      '#description' => t('WARNING: Sorting will cause ALL results to be loaded in memory. May cause heavy load on your server, so use wisely.'),
      '#default_value' => $this->options['sortable'],
      '#options' => array(
        VIEWS_CUSTOMFIELD_SORTABLE_NO => t('No'),
        VIEWS_CUSTOMFIELD_SORTABLE_NUMERIC => t('Numeric'),
        VIEWS_CUSTOMFIELD_SORTABLE_ALPHA => t('Alphanumeric'),
        VIEWS_CUSTOMFIELD_SORTABLE_ALPHA_NOCASE => t('Alphanumeric (case-insensitive)'),
        VIEWS_CUSTOMFIELD_SORTABLE_NAT => t('Natural'),
        VIEWS_CUSTOMFIELD_SORTABLE_NAT_NOCASE => t('Natural (case-insensitive)'),
      ),
    );
  }

  function click_sortable() {
    return $this->options['sortable'] != VIEWS_CUSTOMFIELD_SORTABLE_NO;
  }

  var $sort_order = NULL;
  function click_sort($order) {
    $this->sort_order = $order;
  }

  function pre_execute() {
    // If we're sorting, save the original pager and let Views get all results.
    if ($this->options['sortable'] != VIEWS_CUSTOMFIELD_SORTABLE_NO && $this->sort_order) {
      $this->pager = $this->view->pager; // save the pager
      $this->view->set_items_per_page(0);
    }
  }

  function pre_render($results) {
    // Evaluate the PHP code.
    foreach ($this->view->result as $i => $row) {
      $this->view->result[$i]->{$this->field_alias} = $this->render_row($row);
    }
    
    // If we're sorting, do the actual sorting then fix the results as per the pager info.
    if ($this->options['sortable'] != VIEWS_CUSTOMFIELD_SORTABLE_NO && $this->sort_order) {
      usort($this->view->result, array(new views_customfield_sorter($this->options['sortable'], $this->sort_order, $this->field_alias), 'sort'));

      // Handle paging logic as in view::execute()
      if (!empty($this->pager['items_per_page'])) {

        if (!empty($this->pager['use_pager']) || !empty($this->view->get_total_rows)) {
          $this->view->total_rows = count($this->view->result) - $this->pager['offset'];
        }

        if (!empty($this->pager['use_pager'])) {
          // dump information about what we already know into the globals
          global $pager_page_array, $pager_total, $pager_total_items;
          // total rows in query
          $pager_total_items[$this->pager['element']] = $this->view->total_rows;
          // total pages
          $pager_total[$this->pager['element']] = ceil($pager_total_items[$this->pager['element']] / $this->pager['items_per_page']);

          // What page was requested
          $pager_page_array = isset($_GET['page']) ? explode(',', $_GET['page']) : array();

          // If the requested page was within range. $this->pager['current_page']
          // defaults to 0 so we don't need to set it in an out-of-range condition.
          if (!empty($pager_page_array[$this->pager['element']])) {
            $page = intval($pager_page_array[$this->pager['element']]);
            if ($page > 0 && $page < $pager_total[$this->pager['element']]) {
              $this->pager['current_page'] = $page;
            }
          }
          $pager_page_array[$this->pager['element']] = $this->pager['current_page'];
        }

        $offset = $this->pager['current_page'] * $this->pager['items_per_page'] + $this->pager['offset'];
        $results = array_slice($this->view->result, $offset, $this->pager['items_per_page']);
        $this->view->result = $results;
      }
      
      $this->view->pager = $this->pager;
    }
  }

  function render($values) {
    return $values->{$this->field_alias};
  }

  /**
   * Enter description here...
   *
   * @param stdClass $values
   * @return string
   */
  function render_row($values) {
    $d = $this->view->display[$this->view->current_display];
    $field_handlers = $d->handler->handlers['field'];
    foreach ($field_handlers as $h) {
      // Handle grouped fields
      if (is_a($h, 'content_handler_field_multiple') && $h->defer_query) {
        // content_handler_field_multiple stores its query results in $obj->field_values
        $tmp = array();
        if(!empty($h->field_values[$values->{$h->field_alias}])) {
          foreach ($h->field_values[$values->{$h->field_alias}] as $k => $v) {
            $tmp[$k] = $v;
            // clean up a bit
            unset($tmp[$k]['#delta']);
            unset($tmp[$k]['_nid']);
          }
        }
        $values->{$h->table.'_'.$h->field} = $tmp;
      }
    }
    
    return $this->render_phpcode($this->options['value'], $this->static, $values);
  }

  /**
   * Prepare and evaluate of a string of PHP code.
   * 
   * This method and eval_phpcode are mostly a copy of drupal_eval().
   *
   * @param string $code
   * @param mixed $static
   * @param array $data
   * @return string
   */
  function render_phpcode($code, &$static, $data) {
    global $theme_path, $theme_info, $conf;
  
    // Store current theme path.
    $old_theme_path = $theme_path;
  
    // Restore theme_path to the theme, as long as drupal_eval() executes,
    // so code evaluted will not see the caller module as the current theme.
    // If theme info is not initialized get the path from theme_default.
    if (!isset($theme_info)) {
      $theme_path = drupal_get_path('theme', $conf['theme_default']);
    }
    else {
      $theme_path = dirname($theme_info->filename);
    }
  
    $output = $this->eval_phpcode($code, $static, $data);
  
    // Recover original theme path.
    $theme_path = $old_theme_path;
  
    return $output;
  }

  /**
   * Evaluate a string of PHP code.
   *
   * @param string $code
   * @param mixed $static
   * @param array $data
   * @return string
   */
  function eval_phpcode($code, &$static, $data) {
    ob_start();
    print eval('?>'. $code);
    return ob_get_clean();
  }
}

class views_customfield_sorter {
  var $function;
  var $factor;
  var $field_alias;

  function views_customfield_sorter($sortable, $sort_order, $field_alias) {
    $this->factor = strtoupper($sort_order) == 'ASC' ? 1 : -1;
    $functions = array(
      VIEWS_CUSTOMFIELD_SORTABLE_NUMERIC => create_function('$a,$b', 'return $a-$b;'),
      VIEWS_CUSTOMFIELD_SORTABLE_ALPHA => 'strcmp',
      VIEWS_CUSTOMFIELD_SORTABLE_ALPHA_NOCASE => 'strcasecmp',
      VIEWS_CUSTOMFIELD_SORTABLE_NAT => 'strnatcmp',
      VIEWS_CUSTOMFIELD_SORTABLE_NAT_NOCASE => 'strnatcasecmp',
    );
    $this->function = $functions[$sortable];
    $this->field_alias = $field_alias;
  }

  function sort($a, $b) {
    return $this->factor * call_user_func($this->function, $a->{$this->field_alias}, $b->{$this->field_alias});
  }
}

