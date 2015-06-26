(function ($) {

    Drupal.behaviors.ethnographyBehavior = {
        attach: function (context, settings) {
            $("#text-fr").hide();

            $( "#show-text-en" , context).click(function(e) {
                e.preventDefault();
                $("#text-en").show();
                $("#text-fr").hide();
            });
            $( "#show-text-fr" , context).click(function(e) {
                e.preventDefault();
                $("#text-en").hide();
                $("#text-fr").show();
            });
        }
    };

}(jQuery));
