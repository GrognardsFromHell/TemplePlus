using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace TemplePlusConfig
{
    /// <summary>
    /// Interaction logic for HouseRulesWindow.xaml
    /// </summary>
    public partial class HouseRulesWindow : Window
    {
        public HouseRulesWindow()
        {
            InitializeComponent();

            DataContext = App._iniViewModel;

            if (this.LaxRulesCheckbox.IsChecked.Value){
                this.LaxRulesPanel.Visibility = Visibility.Visible;
            }
            else
            {
                this.LaxRulesPanel.Visibility = Visibility.Collapsed;
            }

            if (this.NonCoreCheckbox.IsChecked.Value) {
                this.NonCorePanel.Visibility = Visibility.Visible;
            }
            else {
                this.NonCorePanel.Visibility = Visibility.Collapsed;
            }
        }

        private void button_Click(object sender, RoutedEventArgs e){
            this.Close();
            
        }

        private void LaxRules_Checked(object sender, RoutedEventArgs e)
        {
            this.LaxRulesPanel.Visibility = Visibility.Visible;
        }

        private void LaxRules_Unchecked(object sender, RoutedEventArgs e)
        {
            this.LaxRulesPanel.Visibility = Visibility.Collapsed;
        }

        private void LaxRules_Initialized(object sender, EventArgs e)
        {

            var chkbx = sender as CheckBox;
           if (chkbx != null && chkbx.IsChecked != null && this.LaxRulesPanel != null){
                if (chkbx.IsChecked == true){
                    LaxRules_Checked(sender, null);
                }
                else
                {
                    LaxRules_Unchecked(sender, null);
                }
            }
            
        }

        private void NonCore_Checked(object sender, RoutedEventArgs e)
        {
            this.NonCorePanel.Visibility = Visibility.Visible;
        }

        private void NonCore_Unchecked(object sender, RoutedEventArgs e)
        {
            this.NonCorePanel.Visibility = Visibility.Collapsed;
        }

        private void NonCore_Initialized(object sender, EventArgs e)
        {
            var box = sender as CheckBox;
            if (box != null && box.IsChecked != null && this.NonCorePanel != null) {
                if (box.IsChecked == true) {
                    NonCore_Checked(sender, null);
                } else {
                    NonCore_Unchecked(sender, null);
                }
            }
        }

        private void CheckBox_Checked_1(object sender, RoutedEventArgs e)
        {

        }

        private void NonCoreSource_Selected(object sender, RoutedEventArgs e)
        {
          var item = e.Source as ListBoxItem;

          if (item != null) {
            var name = item.Name;
            var srcs = App._iniViewModel.NonCoreSources;
            if (!srcs.Contains(name)) {
              srcs.Add(name);
            }
          }
        }

        private void NonCoreSource_Deselected(object sender, RoutedEventArgs e)
        {
          ListBoxItem item = e.Source as ListBoxItem;

          if (item != null) {
            App._iniViewModel.NonCoreSources.RemoveAll(x => x == item.Name);
          }
        }

        private void NonCoreSource_Initialized(object sender, EventArgs e)
        {
          var item = sender as ListBoxItem;

          if (item != null && App._iniViewModel.NonCoreSources.Contains(item.Name)) {
            item.IsSelected = true;
            item.InvalidateVisual();
          }
        }
    }
}
