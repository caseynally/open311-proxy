<?php
/**
 * @copyright 2016 City of Bloomington, Indiana
 * @license http://www.gnu.org/licenses/agpl.txt GNU/AGPL, see LICENSE.txt
 * @param Client   $this->client
 * @param stdClass $this->service             JSON for a single service from GET Service List
 * @param stdClass $this->service_definition  JSON from GET service_definition
 */
use Application\Models\Endpoint;

$endpoint            = $this->client->getEndpoint();
$service_code        = $this->service->service_code;
$service_name        = self::escape($this->service->service_name);
$service_description = self::escape($this->service->description);

foreach (Endpoint::$optionalOpen311Fields as $field) {
	$$field = !empty($_POST[$field]) ? self::escape($_POST[$field]) : '';
}
$embed_uri = self::generateUri('embed.index');
?>
<form method="post" action="<?= $embed_uri; ?>" enctype="multipart/form-data">
    <fieldset><legend>Location</legend>
        <div>
            <span id="location"><?= $address_string; ?></span>
            <button id="openMapButton" class="chooseLocation">Choose Location</button>
        </div>
        <div id="map_overlay"></div>
        <input name="address_string" id="address_string" type="hidden" value="<?= $address_string; ?>" />
        <input name="lat"            id="lat"            type="hidden" value="<?= $lat; ?>" />
        <input name="long"           id="long"           type="hidden" value="<?= $long; ?>" />
    </fieldset>

    <fieldset><legend>Contact Info</legend>
    <?php
        $helper = $this->template->getHelper('field');
        $fields = [
            'first_name' => $this->_('firstname'),
            'last_name'  => $this->_('lastname' ),
            'phone'      => $this->_('phone'    ),
            'email'      => $this->_('email'    )
        ];
        foreach ($fields as $f=>$label) {
            echo $helper->field(['name'=>$f, 'id'=>$f, 'label'=>$label]);
        }

    ?>
    </fieldset>

    <fieldset><legend>Report Details</legend>
        <ul><li><label for="description">Description</label>
                <div><textarea name="description" id="description"><?= $description; ?></textarea></div>
            </li>
        <?php
            if ($this->service_definition && $this->service_definition->attributes) {
                foreach ($this->service_definition->attributes as $attribute) {
                    $code        = self::escape($attribute->code);
                    $description = self::escape($attribute->description);
                    $datatype    = "{$attribute->datatype}";

                    $inputHTML = '';
                    switch ($datatype) {
                        case 'singlevaluelist':
                            echo "
                            <li><label for=\"$code\">$description</label>
                                <div><select name=\"attribute[$code]\" id=\"$code\"><option></option>
                            ";
                                    foreach ($attribute->values as $value) {
                                        $selected = (!empty($_POST['attribute'][$code])
                                                            && $_POST['attribute'][$code] == $value->key)
                                            ? 'selected="selected"'
                                            : '';
                                        $k = self::escape($value->key);
                                        $n = self::escape($value->name);
                                        echo "<option value=\"$k\" $selected>$n</option>";
                                    }
                            echo "
                                    </select>
                                </div>
                            </li>
                            ";
                            break;

                        case 'multivaluelist':
                            echo "
                            <li><fieldset><legend>$description</legend>
                                <ul>
                            ";
                                foreach ($attribute->values as $value) {
                                    $checked = (      isset($_POST['attribute'][$code])
                                                && is_array($_POST['attribute'][$code])
                                                && in_array($value->key, $_POST['attribute'][$code]))
                                        ? 'checked="checked"'
                                        : '';
                                    $k = self::escape($value->key);
                                    $n = self::escape($value->name);
                                    echo "
                                    <li><label>
                                            <input name=\"attribute[{$code}][]\" type=\"checkbox\" value=\"$k\" $checked />
                                            $n
                                        </label>
                                    </li>
                                    ";
                                }
                            echo "
                                </ul>
                                </fieldset>
                            </li>
                            ";
                            break;

                        case 'text':
                            $n =        !empty($_POST['attribute'][$code])
                                ? self::escape($_POST['attribute'][$code])
                                : '';
                            echo "
                            <li><label for=\"$code\">$description</label>
                                <div><textarea name=\"attribute[$code]\" id=\"$code\">$n</textarea></div>
                            </li>
                            ";
                            break;

                        default:
                            $n =        !empty($_POST['attribute'][$code])
                                ? self::escape($_POST['attribute'][$code])
                                : '';
                            echo "
                            <li><label for=\"$code\">$description</label>
                                <div><input name=\"attribute[$code]\" id=\"$code\" value=\"$n\" /></div>
                            </li>
                            ";
                    }
                }
            }
        ?>
        </ul>
    </fieldset>

    <fieldset><legend>Media</legend>
        <p>Upload a photo</p>
        <input type="file" name="media" />
    </fieldset>

    <fieldset>
        <input name="client"       type="hidden" value="<?= $this->client->getId(); ?>" />
        <input name="service_code" type="hidden" value="<?= $service_code; ?>" />

        <?= \Application\Models\Captcha::render(); ?>

        <button type="submit" class="submit"><?= $this->_('submit'); ?></button>
    </fieldset>
</form>
<script type="text/javascript">
<?php
	$lat  = $endpoint->getLatitude()  ? $endpoint->getLatitude()  : DEFAULT_LATITUDE;
	$long = $endpoint->getLongitude() ? $endpoint->getLongitude() : DEFAULT_LONGITUDE;
	echo "
	var DEFAULT_LATITUDE  = $lat,
		DEFAULT_LONGITUDE = $long,
		BASE_URL = '".BASE_URL."';
	";
?>
</script>
<?php
	$this->template->addToAsset('scripts', YUI.'/yui/yui-min.js');
	$this->template->addToAsset('scripts', 'https://maps.googleapis.com/maps/api/js?sensor=true');
	$this->template->addToAsset('scripts', BASE_URL.'/js/map_picker.js');
?>
