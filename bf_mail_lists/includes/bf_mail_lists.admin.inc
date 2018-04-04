<?php

/**
 * @file
 * bf_mail_lists.admin.inc
 */

/**
 * Form constructor.
 */
function bf_mail_lists_form() {
  $form = [];

  $info = 'Preface<br />';
  $info .= '<strong>Process Stand Alone mailing list:</strong>';
  $info .= '<p>If contact belongs to Stand Alone city type and has at least one device without re-rest or test date then tag it.</p>';
  $info .= '<strong>Process 3d Party mailing list:</strong>';
  $info .= '<p>If contact belongs to 3d Party city type and has at least one device without due date then tag it.</p>';

  $form['list_status'] = [
    '#type' => 'container',
    '#attributes' => ['class' => ['bf-mail-lists-status']],
  ];

  $form['explanation'] = [
    '#type' => 'container',
    '#attributes' => ['class' => ['explanation']],
  ];

  $form['explanation']['info'] = [
    '#type' => 'markup',
    '#markup' => $info,
  ];

  $form['create_stand_alone_list'] = [
    '#type' => 'submit',
    '#value' => t('Process Stand Alone mailing list'),
    '#attributes' => ['class' => ['btn-success']],
    '#submit' => ['bf_mail_lists_create_stand_alone_list_submit'],
  ];

  $form['create_third_party_list'] = [
    '#type' => 'submit',
    '#value' => t('Process 3d Party mailing lists'),
    '#attributes' => ['class' => ['btn-success']],
    '#submit' => ['bf_mail_lists_create_third_party_list_submit'],
  ];

  $path = drupal_get_path('module', 'bf_mail_lists');
  $form['#attached']['css'] = ["$path/css/bf-mail-lists.css"];

  $no_retest_ids = variable_get('bf_mail_lists_stand_alone_processed');
  $no_due_ids = variable_get('bf_mail_lists_third_party_processed');

  if ($no_retest_ids || $no_due_ids) {
    ctools_include('modal');
    ctools_include('ajax');
    ctools_modal_add_js();
  }

  if ($no_retest_ids) {
    bf_mail_lists_ctools_popup_style('stand-alone');

    $link = bf_mail_lists_make_link(
      'Create Stand Alone list',
      'create-list/nojs/stand-alone',
      'stand-alone'
    );

    $count_no_retest = count($no_retest_ids);

    $form['stand_alone_no_retest_date_contacts'] = [
      '#type' => 'container',
      '#attributes' => ['class' => ['stand-alone-no-retest-date-contacts']],
    ];

    $form['stand_alone_no_retest_date_contacts']['info'] = [
      '#type' => 'markup',
      '#markup' => "There are $count_no_retest contacts tagged with 'No re-test date available' tag. $link",
    ];
  }

  if ($no_due_ids) {
    bf_mail_lists_ctools_popup_style('third-party');

    $link = bf_mail_lists_make_link(
      'Create 3rd Party list',
      'create-list/nojs/third-party',
      'third-party'
    );

    $count_no_due = count($no_due_ids);

    $form['third_party_no_due_date_contacts'] = [
      '#type' => 'container',
      '#attributes' => ['class' => ['third-party-no-due-date-contacts']],
    ];

    $form['third_party_no_due_date_contacts']['info'] = [
      '#type' => 'markup',
      '#markup' => "There are $count_no_due contacts tagged with 'No due date available' tag. $link",
    ];
  }

  return $form;
}

/**
 * Submit callback.
 */
function bf_mail_lists_create_stand_alone_list_submit($form, &$form_state) {
  $path = drupal_get_path('module', 'bf_mail_lists');
  $contact_ids = bf_mail_lists_get_ids_by_type('stand_alone');

  $ids_chunked = array_chunk($contact_ids, BF_MAIL_LISTS_ITEMS_PER_OPERATION);
  $items_amount = count($contact_ids);

  $operations = [];
  foreach ($ids_chunked as $ids) {
    $operations[] = ['bf_mail_lists_process_stand_alone_list', [$ids, $items_amount]];
  }

  $batch = [
    'title' => t('Tagging contacts with Stand Alone type.'),
    'init_message' => t("Tagging contacts according to city type is starting..."),
    'finished' => 'bf_mail_lists_stand_alone_finished_callback',
    'progress_message' => t('Processed @current out of @total operations.'),
    'operations' => $operations,
    'file' => "$path/includes/bf_mail_lists.batch.inc",
  ];

  batch_set($batch);
}

/**
 * Submit callback.
 */
function bf_mail_lists_create_third_party_list_submit($form, &$form_state) {
  $path = drupal_get_path('module', 'bf_mail_lists');
  $contact_ids = bf_mail_lists_get_ids_by_type('third_party');

  $ids_chunked = array_chunk($contact_ids, BF_MAIL_LISTS_ITEMS_PER_OPERATION * 2);
  $items_amount = count($contact_ids);

  $operations = [];
  foreach ($ids_chunked as $ids) {
    $operations[] = ['bf_mail_lists_process_third_party_list', [$ids, $items_amount]];
  }

  $batch = [
    'title' => t('Tagging contacts with Stand Alone type.'),
    'init_message' => t("Tagging contacts according to city type is starting..."),
    'finished' => 'bf_mail_lists_third_party_finished_callback',
    'progress_message' => t('Processed @current out of @total operations.'),
    'operations' => $operations,
    'file' => "$path/includes/bf_mail_lists.batch.inc",
  ];

  batch_set($batch);
}

/**
 * Retrieves crm_core_contact`s (bundle - organization) IDs
 * of Stand Alone city_type.
 *
 * city_type = 1 - Stand Alone.
 * city_type = 2 - 3rd Party.
 */
function bf_mail_lists_get_ids_by_type($type) {
  $query = db_select('field_data_field_device_company', 'fdc');
  $query->fields('fdc', ['field_device_company_target_id']);

  $query->join('bf_device', 'd', 'fdc.entity_id = d.device_id');
  $query->join('crm_core_contact', 'c', 'fdc.field_device_company_target_id = c.contact_id');
  $query->join('field_data_field_postal_address', 'ps', 'ps.entity_id = c.contact_id');
  $query->join('bf_submission_detail', 'sd', 'ps.field_postal_address_locality = sd.city');

  switch($type) {
    case 'stand_alone':
      $query->leftJoin('field_data_field_device_re_test_date', 'rtd', 'rtd.entity_id = d.device_id');
      $query->leftJoin('field_data_field_device_test_date', 'td', 'td.entity_id = d.device_id');

      $query->condition('sd.city_type', 1);

      $query->condition('rtd.field_device_re_test_date_value', NULL, 'IS');
      $query->condition('td.field_device_test_date_value', NULL, 'IS');
      break;

    case 'third_party':
      $query->leftJoin('field_data_field_device_dob', 'dob', 'dob.entity_id = d.device_id');

      $query->condition('sd.city_type', 2);

      $query->condition('dob.field_device_dob_value', NULL, 'IS');
      break;
  }

  $query->condition('ps.delta', 0);

  $db_or = db_or();
  $db_or->condition('sd.irrigation', NULL, 'IS');
  $db_or->condition('sd.irrigation', 0);

  $query->condition($db_or)->distinct();

  $ids = $query->execute()->fetchCol();

  return $ids;
}

/**
 * Helper function to make a link.
 */
function bf_mail_lists_make_link($link_text = '', $path, $class_suffix) {
  $class = "ctools-modal-$class_suffix";
  $button = ctools_modal_text_button(t($link_text), $path, t($link_text), $class);

  return '<span class="bf-mail-lists-modal-link">' . $button . '</span>';
}

/**
 * Popup’s settings function.
 */
function bf_mail_lists_ctools_popup_style($type) {
  $popup_style = [
    $type => [
      'modalSize' => [
        'type' => 'fixed',
        'width' => 500,
        'height' => 'auto',
        'addHeight' => 500,
      ],
      'closeText' => '',
      'loadingText' => '',
      'animation' => 'fadeIn',
      'modalTheme' => 'bf_mail_lists',
      'animationSpeed' => 'fast',
    ],
  ];

  drupal_add_js($popup_style, 'setting');
  ctools_add_js('bf-mail-lists', 'bf_mail_lists');
}
