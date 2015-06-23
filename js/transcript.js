(function ($){
    var video = videojs($('#islandora_videojs_html5_api')[0]);

// fire up the plugin
    var transcript = video.transcript()

// attach the widget to the page
    var transcriptContainer = document.querySelector('#transcript');
    transcriptContainer.appendChild(transcript.el());
})(jQuery);
