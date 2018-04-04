<?php

/**
 * @file
 * Batch staff related to bf_mail_lists module.
 */

/**
 * Batch callback. Stand alone.
 */
function bf_mail_lists_process_stand_alone_list($ids, $items_amount, &$context) {
  foreach ($ids as $contact_id) {
    $contact = crm_core_contact_load($contact_id);
    $current_terms = field_get_items('crm_core_contact', $contact, 'field_tags');

    $term = taxonomy_get_term_by_name(BF_MAIL_LISTS_NO_RETEST_DATE, 'tags');
    $term = array_values($term);
    $no_retest_term = $term[0];

    // In case there is no NO_RETEST_DATE tag - add it.
    $tag = bf_mail_lists_tag_checker($current_terms, $no_retest_term);

    if (!$tag['tag_persist']) {
      array_unshift($current_terms, ['tid' => $no_retest_term->tid]);
      $contact->field_tags[LANGUAGE_NONE] = $current_terms;

      field_attach_update('crm_core_contact', $contact);
      entity_get_controller('crm_core_contact')->resetCache([$contact_id]);
      search_api_track_item_change('crm_core_contact', [$contact_id]);
    }

    $context['results'][] = $contact_id;
  }

  $current_count = count($context['results']);
  $context['message'] = "Processed $current_count out of $items_amount contacts.";
}

/**
 * Batch callback. Third party.
 */
function bf_mail_lists_process_third_party_list($ids, $items_amount, &$context) {
  foreach ($ids as $contact_id) {
    $contact = crm_core_contact_load($contact_id);
    $current_terms = field_get_items('crm_core_contact', $contact, 'field_tags');

    $term = taxonomy_get_term_by_name(BF_MAIL_LISTS_NO_DUE_DATE, 'tags');
    $term = array_values($term);
    $no_due_term = $term[0];

    // In case there is no NO_DUE_DATE tag - add it.
    $tag = bf_mail_lists_tag_checker($current_terms, $no_due_term);

    if (!$tag['tag_persist']) {
      array_unshift($current_terms, ['tid' => $no_due_term->tid]);
      $contact->field_tags[LANGUAGE_NONE] = $current_terms;

      field_attach_update('crm_core_contact', $contact);
      entity_get_controller('crm_core_contact')->resetCache([$contact_id]);
      search_api_track_item_change('crm_core_contact', [$contact_id]);
    }

    $context['results'][] = $contact_id;
  }

  $current_count = count($context['results']);
  $context['message'] = "Processed $current_count out of $items_amount contacts.";
}

/**
 * Finish callback. Stand alone.
 */
function bf_mail_lists_stand_alone_finished_callback($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One post processed.', '@count posts processed.');
    watchdog('bf_mail_lists_stand_alone_processed', var_export($results, TRUE));
    variable_set('bf_mail_lists_stand_alone_processed', $results);
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}

/**
 * Finish callback. Third party.
 */
function bf_mail_lists_third_party_finished_callback($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One post processed.', '@count posts processed.');
    watchdog('bf_mail_lists_third_party_processed', var_export($results, TRUE));
    variable_set('bf_mail_lists_third_party_processed', $results);
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}
