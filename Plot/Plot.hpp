#ifndef __PLOT_HPP__
#define __PLOT_HPP__

#include <qwt_plot.h>
#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_magnifier.h>

class Plot : public QwtPlot
{
    Q_OBJECT

    public:

        explicit Plot(QString const& title, QWidget* parent = NULL);
        explicit Plot(QwtText const& title, QWidget* parent = NULL);
        virtual ~Plot(void);

        // Getter
        bool isGridVisible(void) const;
        bool isCrossLineVisible(void) const;

    public slots:

        void setGridVisible(bool visible);
        void setCrossLineVisible(bool visible);

    protected:

        QwtLegend*        legend;
        QwtPlotGrid*      grid;
        QwtPlotMarker*    crossLine;
        QwtPlotPanner*    panner;
        QwtPlotZoomer*    yRightZoomer;
        QwtPlotZoomer*    yLeftZoomer;
        QwtPlotMagnifier* magnifier;
};

#endif /* __PLOT_HPP__ */