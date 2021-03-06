<?php
// $Id: node_teasers.inc,v 1.1.2.1 2010/06/11 22:59:45 gundalo Exp $

/**
 * @file
 * Handle the 'node teaser' override task.
 *
 * This plugin overrides node teasers.
 */

/**
 * Specialized implementation of hook_page_manager_task_tasks(). See api-task.html for
 * more information.
 */
function panels_teasers_node_teasers_page_manager_tasks() {
  return array(
    // This is a 'page' task and will fall under the page admin UI
    'task type' => 'page',

    'title' => t('Node teasers'),

    'admin title' => t('Node teasers'),
    'admin description' => t('When enabled, this overrides the default Drupal behavior for displaying node teasers.  If you add variants, you may use selection criteria such as node type or language or user access to provide different views of nodes. If no variant is selected, the default Drupal node teaser view will be used. '),

    // This is task uses 'context' handlers and must implement these to give the
    // handler data it needs.
    'handler type' => 'context',
    'get arguments' => 'panels_teasers_node_teasers_get_arguments',
    'get context placeholders' => 'panels_teasers_node_teasers_get_contexts',

    // Allow this to be enabled or disabled:
    'disabled' => variable_get('panels_teasers_node_teasers_disabled', TRUE),
    'enable callback' => 'panels_teasers_node_teasers_enable',

    // Enable other drupal hooks for this plugin.
    'use hooks' => TRUE,
  );
}


/**
 * Callback to get arguments provided by this task handler.
 *
 * Since this is the node view and there is no UI on the arguments, we
 * create dummy arguments that contain the needed data.
 */
function panels_teasers_node_teasers_get_arguments($task, $subtask_id) {
  return array(
    array(
      'keyword' => 'node',
      'identifier' => t('Node being viewed'),
      'id' => 1,
      'name' => 'nid',
      'settings' => array(),
    ),
  );
}

/**
 * Callback to get context placeholders provided by this handler.
 */
function panels_teasers_node_teasers_get_contexts($task, $subtask_id) {
  return ctools_context_get_placeholders_from_argument(panels_teasers_node_teasers_get_arguments($task, $subtask_id));
}

/**
 * Callback to enable/disable the page from the UI.
 */
function panels_teasers_node_teasers_enable($cache, $status) {
  variable_set('panels_teasers_node_teasers_disabled', $status);

  // Set a global flag so that the menu routine knows it needs
  // to set a message if enabling cannot be done.
  if (!$status) {
    $GLOBALS['page_manager_enabling_node_view'] = TRUE;
  }
}
