<?php
// $Id: views_handler_field_node_revision_link_revert.inc,v 1.1.2.2 2011/01/10 23:14:26 merlinofchaos Exp $
/**
 * Field handler to present a link to revert a node to a revision
 */
class views_handler_field_node_revision_link_revert extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['node_vid'] = array('table' => 'node', 'field' => 'vid');
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['format'] = 'format';
  }

  function access() {
    return user_access('revert revisions') || user_access('administer nodes');
  }

  function render_link($data, $values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $this->get_value($values, 'nid');
    $node->vid = $this->get_value($values, 'vid');
    $node->uid = $this->get_value($values, 'uid');
    $node->format = $this->get_value($values, 'format');
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    // Current revision cannot be reverted.
    if ($node->vid ==  $this->get_value($values, 'node_vid')) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('revert');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "node/$node->nid/revisions/$node->vid/revert";
    $this->options['alter']['query'] = drupal_get_destination();

    return $text;
  }
}
