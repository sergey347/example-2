
/**
 * BF mail lists. Popup markup.
 */

Drupal.theme.prototype.bf_mail_lists = function () {
  var html = '';
  html += '  <div id="ctools-modal">';
  html += '    <div class="ctools-modal-dialog">';
  html += '      <div class="modal-header">';
  html += '        <a class="close" href="#">';
  html +=            Drupal.CTools.Modal.currentSettings.closeText + Drupal.CTools.Modal.currentSettings.closeImage;
  html += '        </a>';
  html += '        <span id="modal-title" class="modal-title"> </span>';
  html += '      </div>';
  html += '      <div id="modal-content" class="modal-content">';
  html += '      </div>';
  html += '    </div>';
  html += '  </div>';
  return html;
};
