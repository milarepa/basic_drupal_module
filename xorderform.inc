<?php
/**
 * Additional functions supporting
 * xorderform
 * */
/**
 * validate email address.
 * @param element
 *  variable that carries the form element
 * @param form_state
 *  the state of the form
 * */
function xo_email_validate($element, &$form_state) {

    $email = $form_state['input']['email'];
    $message = t('Your Email - @email - is Malformed...', array('@email' => $email));
    $result = valid_email_address($email);
    
    if($result != '1') {
        form_error($element, $message);
    }
}

/**
 * validate phone number
 * @param element
 *  field element
 * @param number
 *  user entered phone number
 * */
function xo_phone_validate($element, &$form_state) {
    
    $exp = '^((\([0-9]{3}\) ?)|([0-9]{3}-))?[0-9]{3}-[0-9]{4}$^';
    $number = $form_state['input']['phone'];
    $message = t('Your phone number - @phone - seems malformed. International numbers must be formatted as (12) 345-6789, and US numbers must be formatted as (123) 456-7890', array('@phone' => $number));
    
    if(preg_match($exp, $number) == 0) {
        form_error($element, $message);
    }
}

/**
 * list of current orders
 * */
function xorderform_orderlist() {
    Global $user;

    $renderable_array = array();
    $role = array();
    
    $role = $user->roles;
    // check roles to make sure who gets to see what.
    if(in_array('manager', $role)) {
        $sql = 'SELECT nid FROM node n WHERE n.type = :type AND n.status = :status ORDER BY n.nid DESC';
        $result = db_query($sql, array(
                                ':type' => 'xorderform_data',
                                ':status' => 1,
                            )
        );
    } else {
        $sql = 'SELECT nid, uid FROM node n WHERE n.uid = :uid AND n.type = :type AND n.status = :status ORDER BY n.nid DESC';
        $result = db_query($sql, array(
                                ':uid' => $user->uid,
                                ':type' => 'xorderform_data',
                                ':status' => 1,
                            )
        );
    }

    $renderable_array['explanation'] = array(
        '#markup' => t("Here is a list of orders."),
    );
  
    foreach ($result as $row) {
        //build node - add full html filter and resave.
        $node = node_load($row->nid);
        $node->body['und'][0]['format'] = 'full_html';
        node_save($node);
        $renderable_array['node_list'][]= node_view($node);
    }
    return $renderable_array;
}

/**
 * Implemetation of hook_mail()
 * @param message_subject
 *  string holding message subject
 * @param message_body
 *  string holding body of email
 * @param flag
 *  boolean value returned
 * */
function xorderform_mail($message_subject, $message_body) {
  // add Capitello email address on production module.
  $my_email = 'benjamin@localhost';
  // Flag
  $flag = 0; // default to false - 1 : passed; 0 : failed -
  
  $my_module = 'xorderform';
  $my_mail_token = 'rFKLOkeid8fE8fejeAKDU3i59fkld';
  
  $from = variable_get('system_mail', $my_email);
  $message = array(
    'id' => $my_module . '_' . $my_mail_token,
    'to' => $my_email,
    'subject' => $message_subject,
    'body' => array($message_body),
    'headers' => array(
      'From' => $from,
      'Sender' => $from,
      'Return-Path' => $from,
      'MIME-Version' => '1.0',
      'Content-Type' => 'text/html; charset=UTF-8; format=flowed; delsp=yes',
      'Content-Transfer-Encoding' => '8Bit',
      'X-Mailer' => 'Drupal',
    ),
  );
  $system = drupal_mail_system($my_module, $my_mail_token);
  

// The format function must be called before calling the mail function. 
  $message = $system->format($message);
  
  if ($system->mail($message)) {
    drupal_set_message('Your order has been made. You will recieve a call by phone to complete the purchase. Thank you.');
    $flag = 1;
  }
  else {
    drupal_set_message('You order failed to send but your data has been saved. please call 555-555-5555 to complete your order.');
  }
  return $flag;
}
